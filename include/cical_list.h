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
	destructor destroy_data;
};

// public list api
struct list *new_list(destructor fn);
void destroy_list(struct list *list);
void list_add(struct list *list, void *data);

// public list iterator api

#define ITERATE(L, X) for (iterator X = new_iterator(L); !end(X); X = next(X))

typedef struct list iterator;
iterator new_iterator(struct list *list);
iterator next(iterator it);
bool end(iterator it);
void *current(iterator it);

#endif
