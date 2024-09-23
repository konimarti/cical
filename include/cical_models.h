#ifndef CICAL_MODELS_H
#define CICAL_MODELS_H

struct param {
	char *name;
	struct list *values; /* list of pointers to char */
};

struct property {
	char *name;
	struct list *params; /* list of pointers to struct param */
	char *value;
};

struct component {
	char *name;
	struct list *prop; /* list of pointers to struct property */
	struct list *comp; /* list of pointers to struct component */
};

#endif
