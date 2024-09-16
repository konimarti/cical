#ifndef CICAL_LIST_H
#define CICAL_LIST_H

#include <stdbool.h>

struct node {
	void *data;
	struct node *next;
};

typedef void (*destructor)(void *);

struct list {
	struct node *head;
};

// public list api
struct list *list_create();
void list_destroy(struct list *list, destructor fn);
void list_add(struct list *list, void *data);

// public list iterator api

#define ITERATE(L, X) for (iterator X = new_iterator(L); !end(X); X = next(X))

typedef struct list iterator;
iterator new_iterator(struct list *list);
iterator next(iterator it);
bool end(iterator it);
void *current(iterator it);

#endif
