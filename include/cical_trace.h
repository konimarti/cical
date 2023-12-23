#ifndef CICAL_TRACE
#define CICAL_TRACE

#ifdef NDEBUG
#define TRACE_ON 0
#else
#define TRACE_ON 1
#endif

#define TRACE_PRINTLN(F, ...)                                         \
	do {                                                          \
		if (TRACE_ON)                                         \
			fprintf(stderr, "%s (%s): " F "\n", __func__, \
				__FILE__, __VA_ARGS__);               \
	} while (false);

#endif

