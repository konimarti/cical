#include "cical_print_json.h"
#include "cical_list.h"
#include "cical_models.h"
#include "cical_time.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char const *const space = " ";

typedef void (*printer)(FILE *f, void *, int);

static void
json_print_escaped_str(FILE *f, char *p)
{
	char c;
	if (p) {
		while ((c = *(p++)) != '\0') {
			if (iscntrl(c)) {
				fprintf(f, "\\%03o", c);
			} else if (c == '\"' || c == '\\') {
				fprintf(f, "\\%c", c);
			} else {
				fprintf(f, "%c", c);
			}
		}
	} else {
		fprintf(f, "\"\"");
		return;
	}
}

static void
json_print_list(FILE *f, struct list *l, int indent, printer fn)
{
	int n = 0;
	if (l && l->head) {
		fprintf(f, "[\n");
		ITERATE(l, arg) {
			if (n) {
				fprintf(f, ",\n");
			}
			fn(f, current(arg), indent + 4);
			++n;
		}
		if (n) {
			fprintf(f, "\n");
		}
		fprintf(f, "%*s]", indent + 2, space);
	} else {
		fprintf(f, "[]");
	}
}

static void
json_print_property(FILE *f, void *obj, int indent)
{
	struct property *p = obj;
	fprintf(f, "%*s{\n", indent, space);
	fprintf(f, "%*s\"name\": ", indent + 2, space);
	fprintf(f, "\"");
	json_print_escaped_str(f, p->name);
	if (p->param && strlen(p->param)) {
		fprintf(f, "\",\n%*s\"param\": ", indent + 2, space);
		fprintf(f, "\"");
		json_print_escaped_str(f, p->param);
	}
	if (p->value && strlen(p->value)) {
		fprintf(f, "\",\n%*s\"value\": ", indent + 2, space);
		fprintf(f, "\"");
		json_print_escaped_str(f, p->value);
	}
	fprintf(f, "\"\n");
	fprintf(f, "%*s}", indent, space);
}

static void
json_print_component(FILE *f, void *obj, int indent)
{
	struct component *c = obj;
	fprintf(f, "%*s{\n", indent, space);
	fprintf(f, "%*s\"name\": \"%s\"", indent + 2, space, c->name);

	fprintf(f, ",\n%*s\"prop\": ", indent + 2, space);
	json_print_list(f, c->prop, indent, json_print_property);

	fprintf(f, ",\n%*s\"components\": ", indent + 2, space);
	json_print_list(f, c->comp, indent, json_print_component);

	fprintf(f, "\n%*s}", indent, space);
}

void
json_print(FILE *f, struct component *c)
{
	fprintf(f, "[\n");
	ITERATE(c->comp, x) {
		json_print_component(f, current(x), 2);
	}
	fprintf(f, "\n]\n");
}
