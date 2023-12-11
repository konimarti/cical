/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 Koni Marti */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* enum cical_type { */
/* 	CICAL_UNKNOWN, */
/*  */
/* 	// iCalendar stream */
/* 	CICAL_OBJECT_VCALENDAR, */
/*  */
/* 	// component types */
/* 	CICAL_COMPONENT_VEVENT, */
/* 	CICAL_COMPONENT_VTODO, */
/* 	CICAL_COMPONENT_VJOURNAL, */
/* 	CICAL_COMPONENT_VFREEBUSY, */
/* 	CICAL_COMPONENT_VTIMEZONE, */
/* 	CICAL_COMPONENT_VALARM, */
/* 	CICAL_COMPONENT_STANDARD, */
/* 	CICAL_COMPONENT_DAYLIGHT, */
/*  */
/* 	// property types */
/* 	CICAL_PROPERTY_CALSCALE, */
/* 	CICAL_PROPERTY_METHOD, */
/* 	CICAL_PROPERTY_PRODID, */
/* 	CICAL_PROPERTY_VERSION, */
/* 	// TODO */
/* }; */

struct property {
	char *name;
	char *param;
	char *value;
	struct property *next;
};

char *dup(const char *c) {
	char *dup = malloc(strlen(c) + 1);
	if (dup) {
		strcpy(dup, c);
	}
	return dup;
}

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

	struct property *ptr = p->next, *tmp;
	while (ptr) {
		tmp = ptr->next;
		free(ptr);
		ptr = tmp;
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
		perror("failed to allocate compnent memory");
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
		struct component *ptr = c->next, *tmp;
		while (ptr) {
			tmp = ptr->next;
			free(ptr);
			ptr = tmp;
		}
	}

	free(c);
}

// prepend component c to dst in O(1)
void add_component(struct component *dst, struct component *c) {
	if (!dst || !c) return;

	if (dst->next == (void *)0) {
		dst->next = c;
	} else {
		c->next = dst->next;
		dst->next = c;
	}
}

// prepend property p to component c in O(1)
void add_property(struct component *c, struct property *p) {
	if (!c || !p) return;

	if (c->prop == (void *)0) {
		c->prop = p;
	} else {
		p->next = c->prop;
		c->prop = p;
	}
}

void print_component(struct component *c, int depth) {
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
			ptr = ptr->next;
		}
	} else {
		printf("%s prop: (no properties)\n", indent);
	}
	if (c->next) {
		print_component(c->next, ++depth);
	}
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
	++ptr;
	while (isspace(*ptr)) {
		++ptr;
	}

	char *param = strchr(buf, ';');
	if (param) {
		*param = '\0';
		++param;
		while (isspace(*param)) {
			++param;
		}
	} else {
		param = ptr + strlen(ptr);
	}

	printf("property-name : %s\n", buf);
	if (param) printf("property-param: %s\n", param);
	printf("property-value: %s\n\n", ptr);

	struct property *p = init_property(buf, param, ptr);
	add_property(c, p);
}

int main(void) {
	char buf[BUF_SIZE];

	struct component *stream[256];
	size_t n = 0;

	struct component *top, *tmp;

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
				add_component(top, tmp);
				printf("pushded again: %s\n", top->name);
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
		print_component(stream[i], 0);
		destroy_component(stream[i]);
	}

	destroy_reader(r);
	destroy_stack(s);
}
