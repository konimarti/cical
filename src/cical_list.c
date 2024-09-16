#include <assert.h>
#include <cical_list.h>
#include <errno.h>
#include <stdlib.h>

iterator
new_iterator(struct list *list)
{
	assert(list);
	struct list it = { .head = list->head };
	return it;
}

iterator
next(iterator it)
{
	if (it.head)
		it.head = it.head->next;
	return it;
}

bool
end(iterator it)
{
	if (it.head) {
		return false;
	} else {
		return true;
	}
}

void *
current(iterator it)
{
	return it.head->data;
}

static struct node *
node_create(void *data)
{
	struct node *n = malloc(sizeof(*n));
	if (!n) {
		/* perror("could not create node."); */
		exit(EXIT_FAILURE);
	}
	n->data = data;
	n->next = 0;
	return n;
}

void
node_destroy(struct node *n)
{
	free(n);
}

struct list *
list_create()
{
	struct list *list = malloc(sizeof(*list));
	if (!list) {
		/* perror("could not create list."); */
		exit(EXIT_FAILURE);
	}
	list->head = 0;
	return list;
}

void
list_destroy(struct list *list, destructor destroy)
{
	if (!list)
		return;

	struct node *n = { 0 };
	while (list->head) {
		n = list->head;
		list->head = n->next;

		if (destroy && n->data)
			destroy(n->data);

		node_destroy(n);
	}
	free(list);
}

void
list_add(struct list *list, void *data)
{
	struct node *n = node_create(data);
	n->next = list->head;
	list->head = n;
}

