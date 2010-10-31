#include <object.h>
#include <environment.h>
#include <instruction.h>
#include <primitive_procedures.h>

#include <array.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct pair *NIL;
struct object *UNSPECIFIC;
struct object *TRUE;
struct object *FALSE;

void
dealloc_obj(struct object *obj)
{
  assert(obj->refcount == 0);
  struct pair *p;
  struct integer *i;
  struct string *st;
  struct symbol *sy;
  struct procedure *proc;
  struct prim_proc *pp;
  struct compound_proc *cp;
  struct code *c;
  struct codeptr *cptr;
  struct environment *e;

  if (obj->dinfo) {
    if (obj->dinfo->src_file) {
      free(obj->dinfo->src_file);
    }
    free(obj->dinfo);
  }

  switch (obj->type->code) {
  case PAIR_TYPE:
    p = container_of(obj, struct pair, obj);
    DEC_REF(p->car);
    DEC_REF(p->cdr);
    free(p);
    break;
  case INTEGER_TYPE:
    i = container_of(obj, struct integer, obj);
    free(i);
    break;
  case STRING_TYPE:
    st = container_of(obj, struct string, obj);
    free(st);
    break;
  case SYMBOL_TYPE:
    sy = container_of(obj, struct symbol, obj);
    free(sy);
    break;
  case PROCEDURE_TYPE:
    proc = container_of(obj, struct procedure, obj);
    switch (proc->type) {
    case COMPOUND:
      cp = container_of(proc, struct compound_proc, proc);
      DEC_REF(cp->params);
      DEC_REF(&cp->code->obj);
      if (cp->env) {
        DEC_REF(&cp->env->obj);
      }
      free(cp);
      break;
    case PRIMITIVE:
      pp = container_of(proc, struct prim_proc, proc);
      free(pp);
      break;
    }
    break;
  case CODE_TYPE:
    c = container_of(obj, struct code, obj);
    dealloc_bytecode(c->stream);
    free(c);
    break;
  case CODEPTR_TYPE:
    cptr = container_of(obj, struct codeptr, obj);
    DEC_REF(&cptr->base->obj);
    free(cptr);
    break;
  case ENVIRONMENT_TYPE:
    e = container_of(obj, struct environment, obj);
    dealloc_env(e);
    break;
  default:
    printf("don't know how to deallocate a %s\n", obj->type->name);
    exit(1);
  }
}

void
init_singleton_objects(void)
{
  NIL = make_pair(NULL, NULL);
  NIL->obj.type = get_type(NIL_TYPE);
  NIL->obj.refcount = -1;
  UNSPECIFIC = malloc(sizeof(struct object));
  UNSPECIFIC->type = get_type(UNSPECIFIC_TYPE);
  UNSPECIFIC->refcount = -1;
  TRUE = malloc(sizeof(struct object));
  TRUE->type = get_type(BOOLEAN_TYPE);
  TRUE->refcount = -1;
  FALSE = malloc(sizeof(struct object));
  FALSE->type = get_type(BOOLEAN_TYPE);
  FALSE->refcount = -1;

  env_define(global_env, "unspecific", UNSPECIFIC);
}

struct integer*
make_integer(int x)
{
  struct integer *ret = malloc(sizeof(struct integer));
  ret->obj.type = get_type(INTEGER_TYPE);
  ret->obj.refcount = 0;
  ret->obj.dinfo = NULL;
  ret->value = x;
  return ret;
}

struct string*
make_string(char *str)
{
  struct string *ret = malloc(sizeof(struct string));
  ret->obj.type = get_type(STRING_TYPE);
  ret->obj.refcount = 0;
  ret->obj.dinfo = NULL;
  ret->value = str;
  return ret;
}

struct symbol*
make_symbol(char *str)
{
  struct symbol *ret = malloc(sizeof(struct symbol));
  ret->obj.type = get_type(SYMBOL_TYPE);
  ret->obj.refcount = 0;
  ret->obj.dinfo = NULL;
  ret->value = str;
  return ret;
}

struct pair*
make_pair(struct object *car, struct object *cdr)
{
  struct pair *ret = malloc(sizeof(struct pair));
  ret->obj.type = get_type(PAIR_TYPE);
  ret->obj.refcount = 0;
  ret->obj.dinfo = NULL;
  ret->car = car;
  ret->cdr = cdr;
  if (car) {
    INC_REF(car);
  }
  if (cdr) {
    INC_REF(cdr);
  }
  return ret;
}

struct compound_proc*
make_compound_procedure(struct object *params,
                        struct code *code,
                        struct environment *env)
{
  struct compound_proc *ret = malloc(sizeof(struct compound_proc));
  ret->proc.type = COMPOUND;
  ret->proc.obj.type = get_type(PROCEDURE_TYPE);
  ret->proc.obj.refcount = 0;
  ret->proc.obj.dinfo = NULL;
  ret->params = params;
  ret->code = code;
  ret->env = env;
  INC_REF(params);
  INC_REF(&code->obj);
  if (env) {
    INC_REF(&env->obj);
  }
  return ret;
}

struct prim_proc*
make_primitive_procedure(void *func,
                         unsigned int arity,
                         const char *name,
                         unsigned int takes_ctx)
{
  struct prim_proc *ret = malloc(sizeof(struct prim_proc));
  ret->proc.type = PRIMITIVE;
  ret->proc.obj.type = get_type(PROCEDURE_TYPE);
  ret->proc.obj.refcount = 0;
  ret->proc.obj.dinfo = NULL;
  ret->arity = arity;
  ret->func = func;
  ret->name = name;
  ret->takes_ctx = takes_ctx;
  return ret;
}

struct code*
make_code(struct instruction *stream)
{
  struct code *ret;
  ret = (struct code*) malloc(sizeof(struct code));
  ret->obj.type = get_type(CODE_TYPE);
  ret->obj.refcount = 0;
  ret->obj.dinfo = NULL;
  ret->stream = stream;
  return ret;
}

struct codeptr*
make_codeptr(struct code *base, size_t offset)
{
  struct codeptr *ret;
  ret = (struct codeptr*) malloc(sizeof(struct codeptr));
  ret->obj.type = get_type(CODEPTR_TYPE);
  ret->obj.refcount = 0;
  ret->obj.dinfo = NULL;
  ret->base = base;
  ret->offset = offset;
  INC_REF(&base->obj);
  return ret;
}

struct environment*
make_environment(struct environment *parent)
{
  struct environment *ret = malloc(sizeof(struct environment));
  ret->obj.type = get_type(ENVIRONMENT_TYPE);
  ret->obj.refcount = 0;
  ret->obj.dinfo = NULL;
  str_array_create(&ret->names);
  obj_array_create(&ret->values);
  ret->parent = parent;
  if (parent) {
    INC_REF(&parent->obj);
  }
  return ret;
}

struct object*
print_obj(struct object *obj)
{
  struct pair *next;
  struct procedure *proc;

  switch (obj->type->code) {
  case NIL_TYPE:
    printf("()");
    break;
  case UNSPECIFIC_TYPE:
    printf("<unspecified>");
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
    printf("%d", container_of(obj, struct integer, obj)->value);
    break;
  case SYMBOL_TYPE:
    printf("%s", container_of(obj, struct symbol, obj)->value);
    break;
  case STRING_TYPE:
    printf("\"%s\"", container_of(obj, struct string, obj)->value);
    break;
  case PAIR_TYPE:
    next = container_of(obj, struct pair, obj);
    printf("(");
    while (1) {
      print_obj(next->car);
      if (next->cdr == &NIL->obj) {
        break;
      } else if (next->cdr->type->code != PAIR_TYPE) {
        printf(" . ");
        print_obj(next->cdr);
        break;
      } else {
        printf(" ");
        next = container_of(next->cdr, struct pair, obj);
      }
    }
    printf(")");
    break;
  case PROCEDURE_TYPE:
    proc = container_of(obj, struct procedure, obj);
    switch (proc->type) {
    case PRIMITIVE:
      printf("#<primitive-procedure: %s>",
             container_of(proc, struct prim_proc, proc)->name);
      break;
    case COMPOUND:
      printf("#<procedure>");
      break;
    }
    break;
  case ENVIRONMENT_TYPE:
    printf("#<environment>");
    break;
  default:
    printf("\nError: can't print obj type '%s'\n", obj->type->name);
  }
  return UNSPECIFIC;
}
