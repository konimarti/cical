#include "cical_print_json.h"
#include "cical_list.h"
#include "cical_models.h"
#include "cical_time.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char const *const space = " ";

void
json_print_escaped_str(char *p)
{
	char c;
	if (p) {
		while ((c = *(p++)) != '\0') {
			if (iscntrl(c)) {
				printf("\\%03o", c);
			} else if (c == '\"' || c == '\\') {
				printf("\\%c", c);
			} else {
				printf("%c", c);
			}
		}
	} else {
		printf("\"\"");
		return;
	}
}
typedef void (*printer)(void *, int);

void
json_print_list(struct list *l, int indent, printer fn)
{
	int n = 0;
	if (l && l->head) {
		printf("[\n");
		ITERATE(l, arg) {
			if (n) {
				printf(",\n");
			}
			fn(current(arg), indent + 4);
			++n;
		}
		if (n) {
			printf("\n");
		}
		printf("%*s]", indent + 2, space);
	} else {
		printf("[]");
	}
}

void
json_print_property(void *obj, int indent)
{
	struct property *p = obj;
	printf("%*s{\n", indent, space);
	printf("%*s\"name\": ", indent + 2, space);
	printf("\"");
	json_print_escaped_str(p->name);
	if (p->param && strlen(p->param)) {
		printf("\",\n%*s\"param\": ", indent + 2, space);
		printf("\"");
		json_print_escaped_str(p->param);
	}
	if (p->value && strlen(p->value)) {
		printf("\",\n%*s\"value\": ", indent + 2, space);
		printf("\"");
		json_print_escaped_str(p->value);
	}
	printf("\"\n");
	printf("%*s}", indent, space);
}

void
json_print_component(void *obj, int indent)
{
	struct component *c = obj;
	printf("%*s{\n", indent, space);
	printf("%*s\"name\": \"%s\"", indent + 2, space, c->name);

	printf(",\n%*s\"prop\": ", indent + 2, space);
	json_print_list(c->prop, indent, json_print_property);

	printf(",\n%*s\"components\": ", indent + 2, space);
	json_print_list(c->comp, indent, json_print_component);

	printf("\n%*s}", indent, space);
}

void
json_print(struct component *c)
{
	printf("[\n");
	ITERATE(c->comp, x) {
		json_print_component(current(x), 2);
	}
	printf("\n]\n");
}
