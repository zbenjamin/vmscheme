#include <primitive_procedures.h>

#include <environment.h>
#include <object.h>
#include <type.h>

#include <stdio.h>
#include <stdlib.h>

#define DEF_PRIM(name, func, arity)                             \
  env_define(global_env, name,                                  \
             make_primitive_procedure(func, arity, name))

void
init_primitive_procs()
{
  DEF_PRIM("cons", make_pair, 2);
  DEF_PRIM("car", car, 1);
  DEF_PRIM("cdr", cdr, 1);
  DEF_PRIM("plus", plus, 2);
  DEF_PRIM("minus", minus, 2);
}

struct object*
car(struct object *pair)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for car: %s\n", pair->type->name);
    exit(1);
  }
  return ((struct object**) pair->value)[0];
}

struct object*
cdr(struct object *pair)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for cdr: %s\n", pair->type->name);
    exit(1);
  }
  return ((struct object**) pair->value)[1];
}

struct object*
reverse_list(struct object *lst)
{
  if (lst->type->code != PAIR_TYPE) {
    printf("Wrong type for reverse_list: %s\n", lst->type->name);
    exit(1);
  }

  struct object *ret = NIL;
  struct object *next = lst;
  while (1) {
    ret = make_pair(car(next), ret);
    if (cdr(next) == NIL) {
      break;
    } else if (cdr(next)->type->code != PAIR_TYPE) {
      printf("Improper list passed to reverse_list\n");
      exit(1);
    } else {
      next = cdr(next);
    }
  }

  return ret;
}

struct object*
list_length(struct object *lst)
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

  return make_integer(count);
}

struct object*
plus(struct object *n1, struct object *n2)
{
  struct object *res;
  res = make_integer(n1->ival + n2->ival);
  return res;
}

struct object*
minus(struct object *n1, struct object *n2)
{
  struct object *res;
  res = make_integer(n1->ival - n2->ival);
  return res;
}
