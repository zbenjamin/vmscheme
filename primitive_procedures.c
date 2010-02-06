#include <primitive_procedures.h>

#include <environment.h>
#include <object.h>
#include <type.h>

#include <stdio.h>
#include <stdlib.h>

void
init_primitive_procs()
{
  env_define(global_env, "car", make_primitive_procedure(car, 1));
  env_define(global_env, "cdr", make_primitive_procedure(cdr, 1));
  env_define(global_env, "plus", make_primitive_procedure(plus, 2));
  env_define(global_env, "minus", make_primitive_procedure(minus, 2));
}

struct object*
car(struct object *pair)
{
  if (pair->type != pair_type) {
    printf("Wrong type for car: %s\n", pair->type->name);
    exit(1);
  }
  return ((struct object**) pair->value)[0];
}

struct object*
cdr(struct object *pair)
{
  if (pair->type != pair_type) {
    printf("Wrong type for cdr: %s\n", pair->type->name);
    exit(1);
  }
  return ((struct object**) pair->value)[1];
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
