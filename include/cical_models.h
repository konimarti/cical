#ifndef CICAL_MODELS_H
#define CICAL_MODELS_H

struct property {
	char *name;
	char *param;
	char *value;
};

struct component {
	char *name;
	struct list *prop;
	struct list *comp;
};

#endif
