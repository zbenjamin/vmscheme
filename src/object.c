#include <object.h>
#include <environment.h>
#include <primitive_procedures.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct object *NIL;
struct object *TRUE;
struct object *FALSE;

void
_maybe_dealloc_obj(struct object *obj)
{
  assert(obj->refcount != 0);
  if (obj->refcount > 0) {
    --(obj->refcount);
    if (obj->refcount == 0) {
      dealloc_obj(obj);
    }
  }
}

void
dealloc_obj(struct object *obj)
{
  assert(obj->refcount == 0);
  struct object *ocar;
  struct object *ocdr;

  printf("deallocating %s obj %p: ", obj->type->name, obj);

  switch (obj->type->code) {
  case PAIR_TYPE:
    ocar = car(obj);
    ocdr = cdr(obj);
    printf("%p %p\n", ocar, ocdr);
    DEC_REF(ocar);
    DEC_REF(ocdr);
    free(obj->pval);
    break;
  case INTEGER_TYPE:
    printf("%d\n", obj->ival);
    break;
  case STRING_TYPE:
    printf("%s\n", obj->sval);
    free(obj->sval);
    break;
  case SYMBOL_TYPE:
    printf("%s\n", obj->sval);
    free(obj->sval);
    break;
  case PROCEDURE_TYPE:
    printf("%p %p %p\n", obj->proc_val->params, obj->proc_val->code,
           obj->proc_val->env);
    DEC_REF(obj->proc_val->params);
    DEC_REF(obj->proc_val->code);
    if (obj->proc_val->env) {
      DEC_REF(obj->proc_val->env);
    }
    free(obj->proc_val);
    break;
  case PRIMITIVE_PROC_TYPE:
    printf("%s\n", obj->pproc_val->name);
    free(obj->pproc_val);
    break;
  case CODE_TYPE:
    printf("\n");
    dealloc_bytecode(obj->cval);
    break;
  case ENVIRONMENT_TYPE:
    printf("%p\n", obj->eval->parent);
    dealloc_env(obj);
    break;
  default:
    printf("don't know how to deallocate a %s\n", obj->type->name);
    exit(1);
  }
  free(obj);
}

void
init_singleton_objects()
{
  NIL = malloc(sizeof(struct object));
  NIL->type = get_type(NIL_TYPE);
  NIL->refcount = -1;
  TRUE = malloc(sizeof(struct object));
  TRUE->type = get_type(BOOLEAN_TYPE);
  TRUE->refcount = -1;
  FALSE = malloc(sizeof(struct object));
  FALSE->type = get_type(BOOLEAN_TYPE);
  FALSE->refcount = -1;
}

struct object*
make_integer(int x)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(INTEGER_TYPE);
  ret->ival = x;
  ret->refcount = 0;
  return ret;
}

struct object*
make_string(char *str)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(STRING_TYPE);
  ret->sval = str;
  ret->refcount = 0;
  return ret;
}

struct object*
make_symbol(char *str)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(SYMBOL_TYPE);
  ret->sval = str;
  ret->refcount = 0;
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
  ret->refcount = 0;
  INC_REF(car);
  INC_REF(cdr);
  return ret;
}

struct object*
make_procedure(struct object *params,
               struct object *code,
               struct object *env)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(PROCEDURE_TYPE);
  struct proc_rec *rec = malloc(sizeof(struct proc_rec));
  rec->params = params;
  rec->code = code;
  rec->env = env;
  ret->proc_val = rec;
  ret->refcount = 0;
  INC_REF(params);
  INC_REF(code);
  if (env) {
    INC_REF(env);
  }
  return ret;
}

struct object*
make_primitive_procedure(void *func,
                         unsigned int arity,
                         const char *name,
                         unsigned int takes_ctx)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(PRIMITIVE_PROC_TYPE);
  struct primitive_proc_rec *rec =
    malloc(sizeof(struct primitive_proc_rec));
  rec->arity = arity;
  rec->func = func;
  rec->name = name;
  rec->takes_ctx = takes_ctx;
  ret->pproc_val = rec;
  ret->refcount = 0;
  return ret;
}

struct object*
make_code(struct instruction *code)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(CODE_TYPE);
  ret->cval = code;
  ret->refcount = 0;
  return ret;
}

struct object*
make_environment(struct object *parent)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = get_type(ENVIRONMENT_TYPE);
  struct environment *env = malloc(sizeof(struct environment));
  env->names = 0;
  env->values = 0;
  env->size = 0;
  env->parent = parent;
  ret->eval = env;
  ret->refcount = 0;
  if (parent) {
    INC_REF(parent);
  }
  return ret;
}

struct object*
print_obj(struct object *obj)
{
  struct object *next;

  switch (obj->type->code) {
  case NIL_TYPE:
    printf("()");
    break;
  case BOOLEAN_TYPE:
    if (obj == TRUE) {
      printf("#t");
    } else if (obj == FALSE) {
      printf("#f");
    } else {
      printf("Internal error: invalid boolean\n");
      exit(1);
    }
    break;
  case INTEGER_TYPE:
    printf("%d", obj->ival);
    break;
  case SYMBOL_TYPE:
    printf("%s", obj->sval);
    break;
  case STRING_TYPE:
    printf("\"%s\"", obj->sval);
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
  case PROCEDURE_TYPE:
    printf("#<procedure>");
    break;
  case ENVIRONMENT_TYPE:
    printf("#<environment>");
    break;
  default:
    printf("\nError: can't print obj type '%s'\n", obj->type->name);
  }
  return NIL;
}
