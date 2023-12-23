/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 Koni Marti */
#ifndef CICAL_TIME
#define CICAL_TIME

#include <time.h>

// parses the next rfc5545 datetime (19970610T172345Z) from s and stores it as
// time_t in t. The next position to parse another value is returned. If an
// error occurs a (void*)0 pointer is returned and t is set to -1.
const char* parse_rfc5545_time(const char* s, time_t* t);

#endif
