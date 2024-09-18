#define _XOPEN_SOURCE
#include <cical_time.h>
#include <cical_trace.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RFC5545_DATETIME_FMT "%Y%m%dT%H%M%S"

// TODO: consider VTIMEZONE component
const char *
parse_rfc5545_time(const char *s, time_t *t)
{
	struct tm broken_time = { 0 };

	if (!s || !t)
		goto errReturn;

	char *ptr = strptime(s, RFC5545_DATETIME_FMT, &broken_time);

	if (!ptr)
		goto errReturn;

	if (toupper(*ptr) == 'Z') {
		/* TRACE_PRINTLN("parse UTC time: %s\n", s); */
		++ptr;
		tzset();
		*t = mktime(&broken_time) - timezone;
	} else {
		/* TRACE_PRINTLN("parse local time: %s", s); */
		*t = mktime(&broken_time);
	}

	return ptr;

errReturn:
	*t = -1;
	return (void *)0;
}
