#ifndef CICAL_STACK_H
#define CICAL_STACK_H

#include <stdbool.h>

#define MAX_STACK_SIZE 32

struct stack {
	void *buf[MAX_STACK_SIZE];
	int top;
};

struct stack *init_stack();
void destroy_stack(struct stack *s);
void stack_push(struct stack *s, void *arg);
void *stack_pop(struct stack *s);
bool stack_empty(struct stack *s);

#endif
