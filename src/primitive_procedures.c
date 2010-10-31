#include <primitive_procedures.h>

#include <environment.h>
#include <eval.h>
#include <load.h>
#include <object.h>
#include <parser_aux.h>
#include <type.h>
#include <utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct object*
the_global_environment(void)
{
  return &global_env->obj;
}

#define DEF_PRIM(name, func, arity, takes_ctx)            \
  env_define(global_env, name,                            \
             &make_primitive_procedure(func, arity,       \
                                       name, takes_ctx)->proc.obj)

void
init_primitive_procs(void)
{
  DEF_PRIM("cons", make_pair, 2, 0);
  DEF_PRIM("car", car, 1, 0);
  DEF_PRIM("cdr", cdr, 1, 0);
  DEF_PRIM("set-car!", set_car, 2, 0);
  DEF_PRIM("set-cdr!", set_cdr, 2, 0);
  DEF_PRIM("+", plus, 2, 0);
  DEF_PRIM("-", minus, 2, 0);
  DEF_PRIM("*", mult, 2, 0);
  DEF_PRIM("/", idiv, 2, 0);
  DEF_PRIM("=", iequal, 2, 0);
  DEF_PRIM("eq?", eq_p, 2, 0);
  DEF_PRIM("eqv?", eqv_p, 2, 0);
  DEF_PRIM("object-type", object_type, 1, 0);
  DEF_PRIM("reverse", reverse_list_wrap, 1, 0);
  DEF_PRIM("last-pair", last_pair, 1, 0);
  DEF_PRIM("eval", eval, 2, 0);
  DEF_PRIM("apply", apply_wrap, 2, 1);
  DEF_PRIM("the-global-environment", the_global_environment, 0, 0);
  DEF_PRIM("display", display, 1, 0);
  DEF_PRIM("read", parse_interactive, 0, 0);
  DEF_PRIM("disassemble", disassemble_wrap, 1, 0);
  DEF_PRIM("load", load_wrap, 1, 1);
  DEF_PRIM("string=?", stringeq_p, 2, 0);
}

struct object*
car(const struct object *pair)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for car: %s\n", pair->type->name);
    exit(1);
  }
  return container_of(pair, struct pair, obj)->car;
}

struct object*
cdr(const struct object *pair)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for cdr: %s\n", pair->type->name);
    exit(1);
  }
  return container_of(pair, struct pair, obj)->cdr;
}

struct object*
set_car(struct object *pair, struct object *val)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for set-car!: %s\n", pair->type->name);
    exit(1);
  }
  struct pair *p = container_of(pair, struct pair, obj);
  DEC_REF(p->car);
  INC_REF(val);
  p->car = val;
  return UNSPECIFIC;
}

struct object*
set_cdr(struct object *pair, struct object *val)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Wrong type for set-cdr!: %s\n", pair->type->name);
    exit(1);
  }
  struct pair *p = container_of(pair, struct pair, obj);
  DEC_REF(p->cdr);
  INC_REF(val);
  p->cdr = val;
  return UNSPECIFIC;
}

struct object*
reverse_list_wrap(struct object *lst)
{
  if (lst->type->code != PAIR_TYPE && lst->type->code != NIL_TYPE) {
    printf("Wrong type for reverse_list: %s\n", lst->type->name);
    exit(1);
  }

  return &reverse_list(container_of(lst, struct pair, obj))->obj;
}

struct object*
last_pair(struct object *pair)
{
  if (pair->type->code != PAIR_TYPE) {
    printf("Argument 1 to last-pair is not a pair\n");
    exit(1);
  }

  struct object *next = pair;
  while (cdr(next)->type->code == PAIR_TYPE) {
    next = cdr(next);
  }
  return next;
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

  struct integer *res;
  res = make_integer(container_of(n1, struct integer, obj)->value
                     + container_of(n2, struct integer, obj)->value);
  return &res->obj;
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

  struct integer *res;
  res = make_integer(container_of(n1, struct integer, obj)->value
                     - container_of(n2, struct integer, obj)->value);
  return &res->obj;
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

  struct integer *res;
  res = make_integer(container_of(n1, struct integer, obj)->value
                     * container_of(n2, struct integer, obj)->value);
  return &res->obj;
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

  struct integer *res;
  res = make_integer(container_of(n1, struct integer, obj)->value
                     / container_of(n2, struct integer, obj)->value);
  return &res->obj;
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

  if (container_of(n1, struct integer, obj)->value
      == container_of(n2, struct integer, obj)->value) {
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
  if (o1->type->code == INTEGER_TYPE
      && o2->type->code == INTEGER_TYPE) {
    return iequal(o1, o2);
  }
  return eq_p(o1, o2);
}

struct object*
object_type(struct object *obj)
{
  return &make_integer(obj->type->code)->obj;
}

struct object*
display(struct object *obj)
{
  if (obj->type->code == STRING_TYPE) {
    printf("%s", container_of(obj, struct string, obj)->value);
  } else {
    print_obj(obj);
  }
  return UNSPECIFIC;
}

struct object*
stringeq_p(struct object *o1, struct object *o2) {
  if (o1->type->code != STRING_TYPE) {
    printf("Argument 1 to string=? is not an string\n");
    exit(1);
  }
  if (o2->type->code != STRING_TYPE) {
    printf("Argument 2 to string=? is not an string\n");
    exit(1);
  }

  if (strcmp(container_of(o1, struct string, obj)->value,
             container_of(o2, struct string, obj)->value) == 0) {
    return TRUE;
  }
  return FALSE;
}
