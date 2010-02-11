#include <utils.h>

#include <object.h>
#include <primitive_procedures.h>
#include <type.h>

#include <stdio.h>
#include <stdlib.h>

int
list_length_int(struct object *lst)
{
  if (lst->type->code != PAIR_TYPE) {
    printf("Wrong type for list_length: %s\n", lst->type->name);
    exit(1);
  }

  int count = 0;
  struct object *next = lst;
  while (next != NIL) {
    if (cdr(next)->type->code != PAIR_TYPE
        && cdr(next) != NIL) {
      printf("Improper list passed to list_length\n");
      exit(1);
    }

    next = cdr(next);
    ++count;
  }

  return count;
}
