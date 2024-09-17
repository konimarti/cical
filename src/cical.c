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
	puts("usage: cical [-h] [-v] [-j] [-f FILE]");
	puts("");
	puts("Parse iCalendar streams.");
	puts("");
	puts("options:");
	puts("  -h       show this help message");
	puts("  -v 	 show version");
	puts("  -j 	 print in json format");
	puts("  -f FILE  read from filename (default stdin)");
}

struct property {
	char *name;
	char *param;
	char *value;
};

struct property *
property_create(char *const name, char *const param, char *const value)
{
	struct property *p = malloc(sizeof(*p));
	if (!p) {
		perror("failed to allocate property memory");
		exit(EXIT_FAILURE);
	}

	p->name = name;
	p->param = param;
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
	free(p->param);
	free(p->value);
	free(p);
}

struct component {
	char *name;
	struct list *prop;
	struct list *comp;
};

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

void
print_escaped_json(char *p)
{
	char c;
	while ((c = *(p++)) != '\0') {
		switch (c) {
		case '\\':
		case '"': printf("\\");
		}
		printf("%c", c);
	}
}

void
component_print_json(struct component *c, int depth)
{
	int n;
	char indent[32];
	snprintf(indent, 32, "%*c ", 2 * depth, ' ');

	printf("%s {\n", indent);
	printf("%s  \"name\": \"%s\",\n", indent, c->name);
	printf("%s  \"prop\": [\n", indent);
	if (c->prop) {
		n = 0;
		ITERATE(c->prop, arg) {
			if (n) {
				printf(",\n");
			}

			struct property *ptr = current(arg);

			printf("%s      {\n", indent);
			printf("%s      \"name\": \"", indent);
			print_escaped_json(ptr->name);
			if (ptr->param != NULL) {
				printf("\",\n%s      \"param\": \"", indent);
				print_escaped_json(ptr->param);
			}
			if (ptr->value != NULL) {
				printf("\",\n%s      \"value\": \"", indent);
				print_escaped_json(ptr->value);
			}
			printf("\"}");

			/* printf("%s     \"name=%s param=%s value=%s\",\n", */
			/*        ptr->name, ptr->param, ptr->value); */
			/*
			if (strncmp(ptr->name, "DT", 2) == 0) {
				time_t t = 0;
				const char *parser = ptr->value;
				parser = parse_rfc5545_time(parser, &t);
				TRACE_PRINTLN("time parsed: %s", ctime(&t));
				printf("%s     \"time=%s\",\n", indent,
				       ctime(&t));
			}
			*/
			++n;
		}
		if (n) {
			printf("\n");
		}
	} else {
		printf("%s \"(no properties)\"\n", indent);
	}
	printf("%s   ],\n", indent);
	printf("%s  \"components\": [\n", indent);
	n = 0;
	if (c->comp != NULL) {
		ITERATE(c->comp, arg) {
			if (n) {
				printf(",\n");
			}
			component_print_json(current(arg), ++depth);
			++n;
		}
		if (n) {
			printf("\n");
		}
	}
	printf("%s   ]\n", indent);

	printf("%s }\n", indent);
}

/*
 * Reads until a character in stop is encountered. Does not stop inside of
 * a quoted string.
 *
 */
char *
read_until(char *s, char *stop)
{
	int quotes = 0;
	while (*s && (!strchr(stop, *s) || quotes % 2 == 1)) {
		if (*s == '"')
			quotes++;
		s++;
	}
	return s;
}

char *
read_part(char const *const s, size_t l)
{
	char *ret = malloc((l + 1) * sizeof(*ret));
	memcpy(ret, s, l);
	ret[l] = '\0';
	return ret;
}

struct property *
property_parse(char *const s)
{
	char *name, *param, *value;
	char *start, *end;

	/* read property name */
	start = s;
	end = read_until(start, ":;");
	if (end == 0) {
		fprintf(stderr, "property_parse fail: invalid name\n");
		goto error;
	}
	name = read_part(start, (size_t)(end - start));

	/* read parameter list */
	if (*end == ';') {
		start = ++end;
		end = read_until(start, ":");
		if (end == 0) {
			fprintf(stderr, "property_parse fail: invalid param\n");
			goto error;
		}
		param = read_part(start, (size_t)(end - start));
	} else {
		param = read_part(0, 0);
	}

	/* read value */
	if (*end == ':') {
		end++;
	} else {
		fprintf(stderr, "property_parse fail: no value found\n");
		goto error;
	}
	value = read_part(end, strlen(end));

	return property_create(name, param, value);

error:
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
			if (p)
				component_property_add(top, p);
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
	const char *filename = (void *)0;
	FILE *in_file = (void *)0;
	int c, json;

	json = 0;

	while ((c = getopt(argc, argv, "hvjf:")) != -1) {
		errno = 0;
		switch (c) {
		case 'j': {
			json = 1;
		}; break;
		case 'f': {
			filename = optarg;
		}; break;
		case 'v': {
			version();
			return 1;
		}
		default: {
			usage();
			return 1;
		}
		}
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
			perror("error: cannot open file");
			return 1;
		}
	}

	struct component *top = component_create("top");

	parse_icalendar(in_file, top);

	if (json) {
		printf("[\n");
		ITERATE(top->comp, x) {
			component_print_json(current(x), 1);
		}
		printf("]\n");
	}

	component_destroy(top);

	return 0;
}
