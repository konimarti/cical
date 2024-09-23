#include "cical_print_markdown.h"
#include "cical_list.h"
#include "cical_models.h"
#include "cical_time.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 * TODOs for better markdown rendering:
 * [ ] render links with [..](..)
 * [ ] render mailto: addresses as <..>
 * [ ] render timestamps in human readable form
 * [ ] line breaks at 80
 *
 */

static char const *const hash = "######";

typedef void (*printer)(FILE *f, void *, int);

static void
markdown_print_list(FILE *f, struct list *l, int indent, printer fn)
{
	if (l && l->head) {
		ITERATE(l, arg) {
			fn(f, current(arg), indent);
		}
	}
}

static void
markdown_print_indented_char(FILE *f, void *obj, int indent)
{
	char *p = obj;
	fprintf(f, "    - %s", p);
}

static void
markdown_print_params(FILE *f, void *obj, int indent)
{
	struct param *p = obj;
	fprintf(f, "  - *%s*\n", p->name);
	markdown_print_list(f, p->values, indent, markdown_print_indented_char);
	fprintf(f, "\n");
}

static void
markdown_print_property(FILE *f, void *obj, int indent)
{
	struct property *p = obj;
	if (p->value && strlen(p->value)) {
		fprintf(f, "- *%s*: %s\n", p->name, p->value);
	} else {
		fprintf(f, "- *%s*\n", p->name);
	}
	markdown_print_list(f, p->params, indent + 4, markdown_print_params);
	fprintf(f, "\n");
}

static void
markdown_print_component(FILE *f, void *obj, int indent)
{
	struct component *c = obj;
	fprintf(f, "%.*s %s\n\n", indent + 1, hash, c->name);
	markdown_print_list(f, c->prop, indent, markdown_print_property);
	markdown_print_list(f, c->comp, indent + 1, markdown_print_component);
	fprintf(f, "\n");
}

void
markdown_print(FILE *f, struct component *c)
{
	ITERATE(c->comp, x) {
		markdown_print_component(f, current(x), 0);
	}
}
