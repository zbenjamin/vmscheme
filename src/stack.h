#ifndef VMSCHEME_STACK_H
#define VMSCHEME_STACK_H

#include <object.h>

struct stack {
  struct object** elems;
  unsigned int size;
  unsigned int top;
};

struct stack* make_stack(unsigned int size);
void dealloc_stack(struct stack *stk);
int stack_empty(struct stack *stk);
void stack_push(struct stack *stk, struct object *obj);
struct object* stack_pop(struct stack *stk);

#endif // vMSCHEME_STACK_H
