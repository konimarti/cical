#ifndef CICAL_READER_H
#define CICAL_READER_H

#include <stdio.h>

#define BUF_SIZE 8192

struct reader {
	FILE *in;
	int lines;
	char buf[BUF_SIZE];
};

struct reader *init_reader(FILE *restrict in);
void destroy_reader(struct reader *r);
size_t reader_getline(size_t n, char buf[n], struct reader *r);

#endif