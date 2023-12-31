#include <assert.h>
#include <cical_list.h>
#include <errno.h>
#include <stdlib.h>

iterator new_iterator(struct list *list) {
	assert(list);
	struct list it = {.head = list->head};
	return it;
}

iterator next(iterator it) {
	if (it.head) it.head = it.head->next;
	return it;
}

bool end(iterator it) {
	if (it.head)
		return false;
	else
		return true;
}

void *current(iterator it) { return it.head->data; }

static struct node *new_node(void *data) {
	struct node *n = malloc(sizeof(struct node));
	if (!n) {
		/* perror("could not create node."); */
		exit(EXIT_FAILURE);
	}
	n->data = data;
	n->next = (void *)0;
	return n;
}

struct list *new_list(destructor fn) {
	struct list *list = malloc(sizeof(struct list));
	if (!list) {
		/* perror("could not create list."); */
		exit(EXIT_FAILURE);
	}
	list->head = (void *)0;
	return list;
}

void destroy_list(struct list *list) {
	if (!list) return;
	struct node *current = {0};
	while (list->head) {
		current = list->head;
		list->head = current->next;

		if (list->destroy_data && current->data)
			list->destroy_data(current->data);
	}
	free(list);
}

void list_add(struct list *list, void *data) {
	struct node *n = new_node(data);
	n->next = list->head;
	list->head = n;
}

