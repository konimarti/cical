#include "cical_print_json.h"
#include "cical_list.h"
#include "cical_models.h"

#include <stdio.h>
#include <stdlib.h>

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

void
print_json(struct component *c)
{
	printf("[\n");
	ITERATE(c->comp, x) {
		component_print_json(current(x), 1);
	}
	printf("]\n");
}
