#define _XOPEN_SOURCE
#include <cical_time.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int parse_rfc5545_time(const char* s, struct tm* time) {
	char* ptr;

	if (!s) return 1;

	// DTSTAMP:19970610T172345Z
	ptr = strptime(s, "%Y%m%dT%H%M%S", time);

	if (toupper(*ptr) == 'Z') {
		printf("Parse UTC time: %s\n", s);
		// TODO: how to set UTC in struc tm?
	}

	return 0;
}
