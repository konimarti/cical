#include <cical_reader.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct reader *
reader_create(FILE *restrict in)
{
	struct reader *r = malloc(sizeof(struct reader));
	if (!r) {
		perror("could not create reader");
		exit(EXIT_FAILURE);
	}
	r->in = in;
	r->lines = 0;
	return r;
}

void
reader_destroy(struct reader *r)
{
	if (r)
		free(r);
}

char *
endline(char *s)
{
	while (*s && *s != '\r' && *s != '\n') s++;
	return s;
}

size_t
reader_getline(size_t n, char buf[], struct reader *r)
{
	if (!r || (r && feof(r->in)))
		return 0;

	int ch;
	char *ptr = r->buf;

	while (fgets(ptr, (int)n - (int)(ptr - r->buf), r->in) != NULL) {
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

	ptr = endline(ptr);
	*ptr = '\0';

	strncpy(buf, r->buf, n);
	return strlen(buf);
}

