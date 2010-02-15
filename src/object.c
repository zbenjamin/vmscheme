#include <object.h>
#include <primitive_procedures.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct object *NIL;

void
init_nil()
{
  NIL = malloc(sizeof(struct object));
  NIL->type = get_type(NIL_TYPE);
}

struct object*
make_integer(int x)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(INTEGER_TYPE);
  ret->ival = x;
  return ret;
}

struct object*
make_string(const char *str)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(STRING_TYPE);
  ret->sval = str;
  return ret;
}

struct object*
make_symbol(const char *str)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(SYMBOL_TYPE);
  ret->sval = str;
  return ret;
}

struct object*
make_pair(struct object *car, struct object *cdr)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(PAIR_TYPE);
  ret->pval = malloc(sizeof(struct object*) * 2);
  ret->pval[0] = car;
  ret->pval[1] = cdr;
  ret->refcount = 1;
  INC_REF(car);
  INC_REF(cdr);
  return ret;
}

struct object*
make_procedure(const struct object *params,
               const struct object *body,
               struct environment *env)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(PROCEDURE_TYPE);
  struct proc_rec *rec = malloc(sizeof(struct proc_rec));
  rec->params = params;
  rec->body = body;
  rec->env = env;
  ret->proc_val = rec;
  ret->refcount = 1;
  return ret;
}

struct object*
make_primitive_procedure(void *func,
                         unsigned int arity,
                         const char *name)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(PRIMITIVE_PROC_TYPE);
  struct primitive_proc_rec *rec =
    malloc(sizeof(struct primitive_proc_rec));
  rec->arity = arity;
  rec->func = func;
  rec->name = name;
  ret->pproc_val = rec;
  ret->refcount = 1;
  return ret;
}

void
print_obj(struct object *obj)
{
  struct object *next;

  switch (obj->type->code) {
  case NIL_TYPE:
    printf("'()");
    break;
  case INTEGER_TYPE:
    printf("%d", obj->ival);
    break;
  case SYMBOL_TYPE:
    printf("%s", obj->sval);
    break;
  case PAIR_TYPE:
    next = obj;
    printf("(");
    while (1) {
      print_obj(car(next));
      if (cdr(next) == NIL) {
        break;
      } else if (cdr(next)->type->code != PAIR_TYPE) {
        printf(" . ");
        print_obj(cdr(next));
        break;
      } else {
        printf(" ");
        next = cdr(next);
      }
    }
    printf(")");
    break;
  case PRIMITIVE_PROC_TYPE:
    printf("#<primitive-procedure: %s>",
           obj->pproc_val->name);
    break;
  default:
    printf("\nError: can't print obj type '%s'\n", obj->type->name);
  }
}
