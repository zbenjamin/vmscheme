#include <utils.h>

#include <object.h>
#include <primitive_procedures.h>
#include <type.h>

#include <stdio.h>
#include <stdlib.h>

struct pair*
reverse_list(struct pair *lst)
{
  struct pair *ret = NIL;
  struct pair *next = lst;
  while (next != NIL) {
    if (next->cdr->type->code != PAIR_TYPE
        && next->cdr->type->code != NIL_TYPE) {
      printf("Improper list passed to reverse_list\n");
      exit(1);
    }
    ret = make_pair(next->car, &ret->obj);
    next = container_of(next->cdr, struct pair, obj);
  }

  return ret;
}

unsigned int
list_length(struct pair *lst)
{
  if (lst->obj.type->code != PAIR_TYPE
      && lst->obj.type->code != NIL_TYPE) {
    printf("Wrong type for list_length: %s\n", lst->obj.type->name);
    exit(1);
  }

  unsigned int count = 0;
  struct pair *next = lst;
  while (next != NIL) {
    if (next->cdr->type->code != PAIR_TYPE
        && next->cdr != &NIL->obj) {
      printf("Improper list passed to list_length\n");
      exit(1);
    }

    next = container_of(next->cdr, struct pair, obj);
    ++count;
  }

  return count;
}
