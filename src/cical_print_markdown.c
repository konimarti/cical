#include "cical_print_markdown.h"
#include "cical_list.h"
#include "cical_models.h"
#include "cical_time.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/*
 *
 * TODOs for better markdown rendering:
 * [x] render links with [..](..)
 * [x] render mailto: addresses as <..>
 * [x] render timestamps in human readable form
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

typedef void (*handle_prefix)(FILE *f, char *s);

static void
mailto_handler(FILE *f, char *s)
{
	fprintf(f, "<%s>", s + 7);
}

static void
link_handler(FILE *f, char *s)
{
	fprintf(f, "[link](%s)", s);
}

static void
time_handler(FILE *f, char *s)
{
	time_t t = 0;
	if (parse_rfc5545_time(s, &t) != 0) {
		char *tstr = ctime(&t);
		tstr[strlen(tstr) - 1] = '\0';
		fprintf(f, "%s", tstr);
	} else {
		fprintf(f, "%s", s);
	}
}

enum cical_cmp_prefix_type { CICAL_CMP_PROP_NAME, CICAL_CMP_PROP_VALUE };

struct {
	enum cical_cmp_prefix_type type;
	char *prefix;
	handle_prefix handler;
} value_prefix[] = {
	{ CICAL_CMP_PROP_NAME, "DTSTART", time_handler },
	{ CICAL_CMP_PROP_NAME, "DTEND", time_handler },
	{ CICAL_CMP_PROP_NAME, "DUE", time_handler },
	{ CICAL_CMP_PROP_NAME, "TRIGGER", time_handler },
	{ CICAL_CMP_PROP_NAME, "DTSTAMP", time_handler },
	{ CICAL_CMP_PROP_VALUE, "mailto:", mailto_handler },
	{ CICAL_CMP_PROP_VALUE, "http:", link_handler },
	{ CICAL_CMP_PROP_VALUE, "https:", link_handler },
};

/* static const char *const time_prefix = "DT"; */

static void
markdown_print_property_value(FILE *f, struct property p[static 1], int indent)
{
	char *cmp;
	/* check for certain prefixes */
	size_t value_len = strlen(p->value);
	size_t n = sizeof(value_prefix) / sizeof(value_prefix[0]);
	for (size_t i = 0; i < n; i++) {
		size_t len = strlen(value_prefix[i].prefix);
		if (value_len < len) {
			continue;
		}
		if (value_prefix[i].type == CICAL_CMP_PROP_NAME) {
			cmp = p->name;
		} else if (value_prefix[i].type == CICAL_CMP_PROP_NAME) {
			cmp = p->value;
		} else {
			/* unreachable */
			break;
		}
		if (strncasecmp(cmp, value_prefix[i].prefix, len) == 0) {
			value_prefix[i].handler(f, p->value);
			return;
		}
	}

	/* at this point, we just print the value */
	fprintf(f, "%s", p->value);
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
		fprintf(f, "- *%s*: ", p->name);
		markdown_print_property_value(f, p, indent);
		fprintf(f, "\n");

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
