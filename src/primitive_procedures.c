#include <primitive_procedures.h>

#include <environment.h>
#include <eval.h>
#include <object.h>
#include <type.h>
#include <utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct object*
the_global_environment()
{
  return global_env;
}

#define DEF_PRIM(name, func, arity)                             \
  env_define(global_env, name,                            \
             make_primitive_procedure(func, arity, name))

void
init_primitive_procs()
{
  DEF_PRIM("cons", make_pair, 2);
  DEF_PRIM("car", car, 1);
  DEF_PRIM("cdr", cdr, 1);
  DEF_PRIM("set-car!", set_car, 2);
  DEF_PRIM("set-cdr!", set_cdr, 2);
  DEF_PRIM("+", plus, 2);
  DEF_PRIM("-", minus, 2);
  DEF_PRIM("*", mult, 2);
  DEF_PRIM("/", idiv, 2);
  DEF_PRIM("=", iequal, 2);
  DEF_PRIM("eq?", eq_p, 2);
  DEF_PRIM("eqv?", eqv_p, 2);
  DEF_PRIM("object-type", object_type, 1);
  DEF_PRIM("reverse", reverse_list, 1);
  DEF_PRIM("eval", eval, 2);
  DEF_PRIM("the-global-environment", the_global_environment, 0);
  DEF_PRIM("display", print_obj, 1);
}

struct object*
car(const struct object *pair)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for car: %s\n", pair->type->name);
    exit(1);
  }
  return pair->pval[0];
}

struct object*
cdr(const struct object *pair)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for cdr: %s\n", pair->type->name);
    exit(1);
  }
  return pair->pval[1];
}

struct object*
set_car(struct object *pair, struct object *val)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for set-car!: %s\n", pair->type->name);
    exit(1);
  }
  DEC_REF(pair->pval[0]);
  INC_REF(val);
  pair->pval[0] = val;
  return NIL;
}

struct object*
set_cdr(struct object *pair, struct object *val)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for set-cdr!: %s\n", pair->type->name);
    exit(1);
  }
  DEC_REF(pair->pval[1]);
  INC_REF(val);
  pair->pval[1] = val;
  return NIL;
}

struct object*
reverse_list(struct object *lst)
{
  if (lst->type->code != PAIR_TYPE && lst->type->code != NIL_TYPE) {
    printf("Wrong type for reverse_list: %s\n", lst->type->name);
    exit(1);
  }

  struct object *ret = NIL;
  struct object *next = lst;
  while (next != NIL) {
    if (next->type->code != PAIR_TYPE) {
      printf("Improper list passed to reverse_list\n");
      exit(1);
    }
    ret = make_pair(car(next), ret);
    next = cdr(next);
  }

  return ret;
}

struct object*
list_length(struct object *lst)
{
  return make_integer(list_length_int(lst));
}

struct object*
plus(struct object *n1, struct object *n2)
{
  if (n1->type->code != INTEGER_TYPE) {
    printf("Argument 1 to + is not an integer\n");
    exit(1);
  }
  if (n2->type->code != INTEGER_TYPE) {
    printf("Argument 2 to + is not an integer\n");
    exit(1);
  }

  struct object *res;
  res = make_integer(n1->ival + n2->ival);
  return res;
}

struct object*
minus(struct object *n1, struct object *n2)
{
  if (n1->type->code != INTEGER_TYPE) {
    printf("Argument 1 to - is not an integer\n");
    exit(1);
  }
  if (n2->type->code != INTEGER_TYPE) {
    printf("Argument 2 to - is not an integer\n");
    exit(1);
  }

  struct object *res;
  res = make_integer(n1->ival - n2->ival);
  return res;
}

struct object*
mult(struct object *n1, struct object *n2)
{
  if (n1->type->code != INTEGER_TYPE) {
    printf("Argument 1 to * is not an integer\n");
    exit(1);
  }
  if (n2->type->code != INTEGER_TYPE) {
    printf("Argument 2 to * is not an integer\n");
    exit(1);
  }

  struct object *res;
  res = make_integer(n1->ival * n2->ival);
  return res;
}

struct object*
idiv(struct object *n1, struct object *n2)
{
  if (n1->type->code != INTEGER_TYPE) {
    printf("Argument 1 to / is not an integer\n");
    exit(1);
  }
  if (n2->type->code != INTEGER_TYPE) {
    printf("Argument 2 to / is not an integer\n");
    exit(1);
  }

  struct object *res;
  res = make_integer(n1->ival / n2->ival);
  return res;
}

struct object*
iequal(struct object *n1, struct object *n2)
{
  if (n1->type->code != INTEGER_TYPE) {
    printf("Argument 1 to = is not an integer\n");
    exit(1);
  }
  if (n2->type->code != INTEGER_TYPE) {
    printf("Argument 2 to = is not an integer\n");
    exit(1);
  }

  if (n1->ival == n2->ival) {
    return TRUE;
  }
  return FALSE;
}

struct object*
eq_p(struct object *o1, struct object *o2)
{
  if (o1 == o2) {
    return TRUE;
  }
  return FALSE;
}

struct object*
eqv_p(struct object *o1, struct object *o2)
{
  // XXX
  if (o1->type->code != SYMBOL_TYPE
      || o2->type->code != SYMBOL_TYPE) {
    return FALSE;
  }
  if (strcmp(o1->sval, o2->sval) == 0) {
    return TRUE;
  }
  return FALSE;
}

struct object*
object_type(struct object *obj)
{
  return make_integer(obj->type->code);
}
