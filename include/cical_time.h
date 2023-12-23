#ifndef CICAL_TIME
#define CICAL_TIME

#include <time.h>

// parse_rfc5545_time parses the string s and expectes an initialized struct tm
// object. Returns 0 if sucessful or >0 if an error occured.
int parse_rfc5545_time(const char* s, struct tm* time);

#endif
