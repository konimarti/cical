#ifndef CICAL_PRINT_JSON_H
#define CICAL_PRINT_JSON_H

#include <stdio.h>

struct component;

void json_print(FILE *f, struct component *c);

#endif
