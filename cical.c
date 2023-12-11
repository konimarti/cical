/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 Koni Marti */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum comp_type {
	UNKNOWN,
	VCALENDAR,
	VEVENT,
	VTODO,
	VJOURNAL,
	VFREEBUSY,
	VTIMEZONE,
	VALARM,
	STANDARD,
	DAYLIGHT
};

enum prop_type {
	CALSCALE,
	METHOD,
	PRODID,
	VERSION,
	ATTACH,
	CATEGORIES,
	CLASS,
	COMMENT,
	DESCRIPTION,
	GEO,
	LOCATION
	// TODO
};

struct parameter {
	char *name;
	char *value;
	struct parameter *next;
};

struct property {
	enum prop_type type;
	struct parameter *param;
};

struct component {
	enum comp_type type;
	struct property *prop;
	struct component *comp;
};

enum comp_type find_comp_type(const char type[static 1]) {
	// TODO
	printf("   type: %s\n", type);
	return UNKNOWN;
}

struct component *init_component(const char *type) {
	struct component *c = malloc(sizeof(struct component));
	if (!c) {
		perror("failed to allocate compnent memory");
		exit(EXIT_FAILURE);
	}
	c->type = find_comp_type(type);
	return c;
}

#define MAX_STACK_SIZE 100

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

struct component *stack_top(struct stack *s) {
	if (s) return s->buf[s->top];
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

char *endline(char *buf) {
	char *ptr = strstr(buf, "\r\n");
	if (ptr == (void *)0) {
		ptr = strchr(buf, '\n');
		if (!ptr) {
			return buf + strlen(buf);
		}
	}
	return ptr;
}

size_t reader_getline(size_t n, char buf[n], struct reader *r) {
	if (!r) return 0;
	if (feof(r->in)) return 0;

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

	ptr = endline(r->buf);
	*ptr = '\0';

	strcpy(buf, r->buf);
	return strlen(buf);
}

void parse_property(char *const buf,
		    struct component *const c __attribute__((unused))) {
	char *ptr = buf;
	bool in_quotes = false;

	while (!in_quotes && *ptr != ':') {
		switch (*ptr) {
			case '\0':
				fprintf(stderr,
					"no valid property found in: %s\n",
					buf);
				return;
			case '"':
				in_quotes = !in_quotes;
				break;
		}
		ptr++;
	}

	*ptr = '\0';
	printf("property-name : %s\n", buf);
	printf("property-value: %s\n\n", ptr + 1);
}

int main(void) {
	char buf[BUF_SIZE];

	struct component *comps[256];
	size_t comps_index = 0;

	struct component *top;

	struct stack *s = init_stack();
	struct reader *r = init_reader(stdin);

	while (reader_getline(sizeof(buf), buf, r)) {
		/* printf("read: %s\n", buf); */

		if (strncmp(buf, "BEGIN:", 6) == 0) {
			printf("pushed: %s\n", buf + 6);
			top = init_component(buf + 6);
			stack_push(s, top);
			continue;
		}

		if (strncmp(buf, "END:", 4) == 0) {
			printf("popped: %s\n", buf + 4);
			comps[comps_index++] = stack_pop(s);
			top = (void *)0;
			continue;
		}

		if (top) {
			parse_property(buf, top);
		}
	}

	for (size_t i = 0; i < comps_index; i++) {
		printf("component %d\n", comps[i]->type);
	}

	destroy_reader(r);
	destroy_stack(s);
}
