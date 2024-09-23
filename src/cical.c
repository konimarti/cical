/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 Koni Marti */

#include "cical.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *strdup(const char *);

static void
version()
{
	printf("version: %s\n", VERSION);
}

static void
usage(void)
{
	puts("usage: cical [-h|-v] [-j] [-i FILE] [-o FILE]");
	puts("");
	puts("Parse iCalendar streams.");
	puts("");
	puts("options:");
	puts("  -h       show this help message");
	puts("  -v 	 show version");
	puts("  -j 	 print in json format");
	puts("  -m 	 print in markdown format");
	puts("  -i FILE  read from filename (default stdin)");
	puts("  -o FILE  write to filename (default stdout)");
}

struct param *
param_create()
{
	struct param *p = malloc(sizeof(*p));
	if (!p) {
		perror("failed to allocate property memory");
		exit(EXIT_FAILURE);
	}

	p->name = 0;
	p->values = list_create();
	return p;
}

void
param_destroy(void *arg)
{
	struct param *p = arg;
	if (!p)
		return;
	free(p->name);
	list_destroy(p->values, free);
	free(p);
}

struct property *
property_create(char *const name, struct list *const params, char *const value)
{
	struct property *p = malloc(sizeof(*p));
	if (!p) {
		perror("failed to allocate property memory");
		exit(EXIT_FAILURE);
	}

	p->name = name;
	p->params = params;
	p->value = value;
	return p;
}

void
property_destroy(void *arg)
{
	struct property *p = arg;
	if (!p) {
		return;
	}
	free(p->name);
	list_destroy(p->params, param_destroy);
	free(p->value);
	free(p);
}

void
component_destroy(void *arg)
{
	struct component *c = arg;
	if (!c) {
		return;
	}
	list_destroy(c->prop, property_destroy);
	list_destroy(c->comp, component_destroy);
	free(c->name);
	free(c);
}

struct component *
component_create(const char *name)
{
	struct component *c = malloc(sizeof(struct component));
	if (!c) {
		perror("failed to allocate component memory");
		exit(EXIT_FAILURE);
	}
	c->name = strdup(name);
	c->prop = list_create();
	c->comp = list_create();
	return c;
}

// prepend component c to dst in O(1)
void
component_add(struct component *dst, struct component *c)
{
	if (!dst || !c)
		return;

	list_add(dst->comp, c);
}

// prepend property p to component c in O(1)
void
component_property_add(struct component *c, struct property *p)
{
	if (!c || !p)
		return;

	list_add(c->prop, p);
}

/*
 * Reads until a character in the stop chars or the string end is encountered.
 * Stop chars are ignored in a quoted string.
 *
 * Returns the position in the string at a stopped char or at 0.
 *
 */
static char *
read_until(char *s, char *stop)
{
	bool in_quotes = false;
	while (*s && (!strchr(stop, *s) || in_quotes)) {
		if (*s == '"') {
			in_quotes = !in_quotes;
		}
		s++;
	}
	return s;
}

static char *
read_part(char const *const s, size_t l)
{
	char *ret = malloc((l + 1) * sizeof(*ret));
	memcpy(ret, s, l);
	ret[l] = '\0';
	return ret;
}

/*
 * Parses a contentline according to RFC 5545, section 3.1 with the following
 * grammar:
 *
 * contentline   = name *(";" param ) ":" value CRLF
 *
 */
struct property *
property_parse(char *const s)
{
	char *start, *end;
	char *name, *value;

	struct list *params;
	struct param *p;

	name = 0;
	params = 0;
	value = 0;

	/* read property name */
	start = s;
	end = read_until(start, ":;");
	if (end == start) {
		fprintf(stderr, "property_parse fail: invalid name\n");
		goto error;
	}
	name = read_part(start, (size_t)(end - start));

	/* read parameter list */
	params = list_create();
	if (*end == ';') {
		start = ++end;
		do {
			p = param_create();
			end = read_until(start, "=");
			if (*end != '=') {
				fprintf(stderr,
					"property_parse fail: "
					"invalid param-name: "
					"expected '=' but got '%c'\n",
					*end);
				goto error;
			}
			p->name = read_part(start, (size_t)(end - start));
			TRACE_PRINTLN(
				"param-name: %.*s", (int)(end - start), start);

			do {
				start = ++end;
				end = read_until(start, ",:;");
				if (end == start) {
					fprintf(stderr,
						"property_parse fail: "
						"invalid param-value\n");
					goto error;
				}
				list_add(p->values,
					read_part(
						start, (size_t)(end - start)));
				TRACE_PRINTLN("param-value: %.*s",
					(int)(end - start),
					start);

			} while (*end && *end == ',');

			list_add(params, p);

		} while (*end && *end == ';');
	}

	/* read value */
	if (*end == ':') {
		end++;
	} else {
		fprintf(stderr, "property_parse fail: no value found\n");
		goto error;
	}
	value = read_part(end, strlen(end));

	return property_create(name, params, value);

error:
	free(name);
	list_destroy(params, param_destroy);
	free(value);
	fprintf(stderr, "invalid contentline: %s", s);
	return 0;
}

void
parse_component(struct reader *r, struct component *top)
{
	char buf[BUF_SIZE];
	struct component *child = 0;
	struct property *p = 0;
	while (reader_getline(sizeof(buf), buf, r)) {
		if (strncmp(buf, "BEGIN:", 6) == 0) {
			TRACE_PRINTLN("parse BEGIN: %s", buf + 6);
			child = component_create(buf + 6);
			parse_component(r, child);
			component_add(top, child);
		} else if (strncmp(buf, "END:", 4) == 0) {
			TRACE_PRINTLN("END: %s", buf + 4);
			break;
		} else {
			p = property_parse(buf);
			if (p) {
				component_property_add(top, p);
			}
		}
	}
}

void
parse_icalendar(FILE *f, struct component *top)
{
	assert(f);
	assert(top);

	struct reader *r = reader_create(f);
	parse_component(r, top);
	reader_destroy(r);
}

int
main(int argc, char *argv[])
{
	const char *filename, *outfilename;
	FILE *in_file, *out_file;
	int c, json, markdown;

	filename = 0;
	outfilename = 0;

	in_file = 0;
	out_file = 0;

	json = 0;
	markdown = 0;

	while ((c = getopt(argc, argv, "hvjmi:o:")) != -1) {
		errno = 0;
		switch (c) {
		case 'j': {
			json = 1;
		}; break;
		case 'm': {
			markdown = 1;
		}; break;
		case 'i': {
			filename = optarg;
		}; break;
		case 'o': {
			outfilename = optarg;
		}; break;
		case 'v': {
			version();
			return 1;
		}
		case 'h':
		default: {
			usage();
			return 1;
		}
		}
	}

	if (json && markdown) {
		fprintf(stderr,
			"json and markdown options are mutually exclusive.");
		return 1;
	}

	if (optind < argc) {
		fprintf(stderr,
			"%s: unexpected argument -- '%s'\n",
			argv[0],
			argv[optind]);
		usage();
		return 1;
	}

	if (!filename || !strcmp(filename, "-")) {
		in_file = stdin;
	} else {
		in_file = fopen(filename, "r");
		if (!in_file) {
			perror("error: cannot open file to read");
			return 1;
		}
	}

	if (!outfilename || !strcmp(outfilename, "-")) {
		out_file = stdout;
	} else {
		out_file = fopen(outfilename, "w");
		if (!out_file) {
			perror("error: cannot open file to write");
			return 1;
		}
	}

	struct component *top = component_create("top");
	parse_icalendar(in_file, top);

	if (json) {
		json_print(out_file, top);
	} else if (markdown) {
		markdown_print(out_file, top);
	}

	component_destroy(top);
	return 0;
}
