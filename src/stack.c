#include <stack.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>

struct stack*
make_stack(unsigned int size)
{
  struct stack *ret = malloc(sizeof(struct stack));
  ret->size = size;
  ret->elems = malloc(sizeof(struct object*) * size);
  ret->top = 0;
  return ret;
}

void
dealloc_stack(struct stack *stk)
{
  assert(stack_empty(stk));
  free(stk->elems);
  free(stk);
}

int
stack_empty(struct stack *stk)
{
  return stk->top == 0;
}

void
stack_push(struct stack *stk, struct object *obj)
{
  stk->elems[stk->top++] = obj;
  if (stk->top == stk->size) {
    size_t oldsize = sizeof(struct object*) * stk->size;
    stk->size = stk->size * 2;
    size_t newsize = oldsize * 2;;
    struct object** newelems = malloc(newsize);
    memcpy(newelems, stk->elems, oldsize);
    free(stk->elems);
    stk->elems = newelems;
  }
}

struct object*
stack_pop(struct stack *stk) {
  assert(stk->top > 0);
  return stk->elems[--stk->top];
}
