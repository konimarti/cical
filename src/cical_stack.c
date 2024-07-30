#include <cical_stack.h>
#include <stdlib.h>

struct stack *init_stack() {
	struct stack *s = malloc(sizeof(struct stack));
	if (!s) {
		/* perror("stack allocation failed"); */
		exit(EXIT_FAILURE);
	}
	s->top = -1;
	return s;
}

void destroy_stack(struct stack *s) {
	if (s) free(s);
}

void stack_push(struct stack *s, void *arg) {
	if (s) s->buf[++(s->top)] = arg;
}

void *stack_pop(struct stack *s) {
	if (s) return s->buf[(s->top)--];
	return (void *)0;
}

bool stack_empty(struct stack *s) {
	if (s && s->top < 0) return true;
	return false;
}