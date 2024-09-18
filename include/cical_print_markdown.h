#ifndef CICAL_PRINT_MARKDOWN_H
#define CICAL_PRINT_MARKDOWN_H

#include <stdio.h>

struct component;

void markdown_print(FILE *f, struct component *c);

#endif
