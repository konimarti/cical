/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 Koni Marti */

#include <cical.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void version() { printf("version: %s\n", VERSION); }

static void usage(void) {
	puts("usage: cical [-h] [-v] [-f FILE]");
	puts("");
	puts("Parse iCalendar streams.");
	puts("");
	puts("options:");
	puts("  -h       show this help message");
	puts("  -v 	 show version");
	puts("  -f FILE  read from filename (default stdin)");
}

static char *dup(const char *c) {
	char *dup = malloc(strlen(c) + 1);
	if (dup) {
		strcpy(dup, c);
	}
	return dup;
}

struct property {
	char *name;
	char *param;
	char *value;
	struct property *next;
};

struct property *init_property(const char *name, const char *param,
			       const char *value) {
	struct property *p = malloc(sizeof(struct property));
	if (!p) {
		perror("failed to allocate property memory");
		exit(EXIT_FAILURE);
	}

	p->name = dup(name);
	p->param = dup(param);
	p->value = dup(value);
	p->next = (void *)0;
	return p;
}

void destroy_property(struct property *p) {
	if (!p) return;

	if (p->name) free(p->name);
	if (p->param) free(p->param);
	if (p->value) free(p->value);

	if (p->next) {
		destroy_property(p->next);
	}

	free(p);
}

struct component {
	char *name;
	struct property *prop;
	struct component *next;
};

struct component *init_component(const char *name) {
	struct component *c = malloc(sizeof(struct component));
	if (!c) {
		perror("failed to allocate component memory");
		exit(EXIT_FAILURE);
	}
	c->name = dup(name);
	c->prop = (void *)0;
	c->next = (void *)0;
	return c;
}

void destroy_component(struct component *c) {
	if (!c) return;

	if (c->name) {
		free(c->name);
	}

	if (c->prop) {
		destroy_property(c->prop);
	}

	if (c->next) {
		destroy_component(c->next);
	}

	free(c);
}

// prepend component c to dst in O(1)
void component_add(struct component *dst, struct component *c) {
	if (!dst || !c) return;

	if (dst->next) c->next = dst->next;

	dst->next = c;
}

// prepend property p to component c in O(1)
void component_add_property(struct component *c, struct property *p) {
	if (!c || !p) return;

	if (c->prop) p->next = c->prop;

	c->prop = p;
}

void component_print(struct component *c, int depth) {
	if (!c) return;

	char indent[32];
	snprintf(indent, 32, "%*d>", depth * 2, depth);

	printf("%s name: %s\n", indent, c->name);
	if (c->prop) {
		printf("%s prop:\n", indent);
		struct property *ptr = c->prop;
		while (ptr) {
			printf("%s     name=%s param=%s value=%s\n", indent,
			       ptr->name, ptr->param, ptr->value);
			if (strncmp(ptr->name, "DT", 2) == 0) {
				time_t t = 0;
				const char *parser = ptr->value;
				parser = parse_rfc5545_time(parser, &t);
				TRACE_PRINTLN("time parsed: %s", ctime(&t));
			}
			ptr = ptr->next;
		}
	} else {
		printf("%s prop: (no properties)\n", indent);
	}
	if (c->next) {
		component_print(c->next, ++depth);
	}
}

#define MAX_STACK_SIZE 32

struct stack {
	struct component *buf[MAX_STACK_SIZE];
	int top;
};

struct stack *init_stack() {
	struct stack *s = malloc(sizeof(struct stack));
	if (!s) {
		perror("stack allocation failed");
		exit(EXIT_FAILURE);
	}
	s->top = -1;
	return s;
}

void destroy_stack(struct stack *s) {
	if (s) free(s);
}

void stack_push(struct stack *s, struct component *c) {
	if (s) s->buf[++(s->top)] = c;
}

struct component *stack_pop(struct stack *s) {
	if (s) return s->buf[(s->top)--];
	return (void *)0;
}

bool stack_empty(struct stack *s) {
	if (s && s->top < 0) return true;
	return false;
}

#define BUF_SIZE 8192

struct reader {
	FILE *in;
	int lines;
	char buf[BUF_SIZE];
};

struct reader *init_reader(FILE *restrict in) {
	struct reader *r = malloc(sizeof(struct reader));
	if (!r) {
		perror("could not create reader");
		exit(EXIT_FAILURE);
	}
	r->in = in;
	r->lines = 0;
	return r;
}

void destroy_reader(struct reader *r) {
	if (r) free(r);
}

static char *endline(char *buf) {
	char *ptr = strstr(buf, "\r\n");
	if (!ptr) ptr = strchr(buf, '\n');
	if (!ptr) ptr = buf + strlen(buf);
	return ptr;
}

size_t reader_getline(size_t n, char buf[n], struct reader *r) {
	if (!r || (r && feof(r->in))) return 0;

	int ch;
	char *ptr = r->buf;

	while (fgets(ptr, BUF_SIZE + (int)(ptr - r->buf), r->in) != NULL) {
		++r->lines;

		if ((ch = fgetc(r->in)) == EOF) {
			break;
		}

		if (!isspace(ch)) {
			if (ungetc(ch, r->in) == EOF) {
				perror("could not push character back");
				exit(EXIT_FAILURE);
			}
			break;
		}

		ptr = endline(ptr);
	}

	*endline(r->buf) = '\0';

	strncpy(buf, r->buf, n);
	return strlen(buf);
}

void parse_property(char *const buf, struct component *const c) {
	char *ptr = buf, *t, *param;
	bool has_param = false;

	while (*ptr != ':') {
		switch (*ptr) {
			case '\0':
				fprintf(stderr,
					"no valid property found in: %s\n",
					buf);
				return;
			case ';':
				has_param = true;
				break;
			case '"':
				t = strchr(ptr + 1, '"');
				if (t) ptr = t;
				break;
		}
		ptr++;
	}
	*ptr = '\0';

	while (isspace(*(++ptr)))
		;

	if (has_param) {
		t = strchr(buf, ';');
		*t = '\0';
		param = ++t;
	} else
		param = ptr + strlen(ptr);

	TRACE_PRINTLN("property-name : '%s'", buf);
	TRACE_PRINTLN("property-param: '%s'", param);
	TRACE_PRINTLN("property-value: '%s'\n", ptr);

	component_add_property(c, init_property(buf, param, ptr));
}

void parse_icalendar_stream(FILE *f) {
	char buf[BUF_SIZE];

	struct component *stream[256];
	size_t n = 0;

	struct component *top, *tmp;

	struct stack *s = init_stack();
	struct reader *r = init_reader(f);

	while (reader_getline(sizeof(buf), buf, r)) {
		/* printf("read: %s\n", buf); */

		if (strncmp(buf, "BEGIN:", 6) == 0) {
			TRACE_PRINTLN("stack: push: %s", buf + 6);
			top = init_component(buf + 6);
			stack_push(s, top);
			continue;
		}

		if (strncmp(buf, "END:", 4) == 0) {
			TRACE_PRINTLN("stack: pop: %s", buf + 4);
			if (stack_empty(s)) {
				perror("trying to pop empty stack");
				exit(EXIT_FAILURE);
			}
			tmp = stack_pop(s);
			if (stack_empty(s)) {
				top = (void *)0;
				stream[n++] = tmp;
			} else {
				top = stack_pop(s);
				component_add(top, tmp);
				TRACE_PRINTLN("stack: push back: %s",
					      top->name);
				stack_push(s, top);
			}
			continue;
		}

		if (top) {
			parse_property(buf, top);
		}
	}

	for (size_t i = 0; i < n; i++) {
		printf("object: %ld\n", i);
		component_print(stream[i], 0);
		destroy_component(stream[i]);
	}

	destroy_reader(r);
	destroy_stack(s);
}

int main(int argc, char *argv[]) {
	const char *filename = (void *)0;
	FILE *in_file = (void *)0;
	int c;

	while ((c = getopt(argc, argv, "hvf:")) != -1) {
		errno = 0;
		switch (c) {
			case 'f':
				filename = optarg;
				break;
			case 'v':
				version();
				return 1;
			default:
				usage();
				return 1;
		}
	}
	if (optind < argc) {
		fprintf(stderr, "%s: unexpected argument -- '%s'\n", argv[0],
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
	parse_icalendar_stream(in_file);
	return 0;
}
