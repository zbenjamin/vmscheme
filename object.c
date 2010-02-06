#include <object.h>

#include <stdlib.h>
#include <string.h>

struct object*
make_integer(int x)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = integer_type;
  ret->ival = x;
  return ret;
}

struct object*
make_string(const char *str)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = string_type;
  ret->sval = str;
  return ret;
}

struct object*
make_symbol(const char *str)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = symbol_type;
  ret->sval = str;
  return ret;
}

struct object*
make_pair(struct object *car, struct object *cdr)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = pair_type;
  struct object** val = malloc(sizeof(struct object*) * 2);
  val[0] = car;
  val[1] = cdr;
  ret->value = val;
  return ret;
}

struct object*
make_procedure(const struct object *params,
               const struct object *body,
               struct environment *env)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = procedure_type;
  struct proc_rec *rec = malloc(sizeof(struct proc_rec));
  rec->params = params;
  rec->body = body;
  rec->env = env;
  ret->value = rec;
  return ret;
}

struct object*
make_primitive_procedure(void *func,
                         unsigned int arity)
{
  struct object *ret = malloc(sizeof(struct object));
  ret->type = procedure_type;
  struct primitive_proc_rec *rec =
    malloc(sizeof(struct primitive_proc_rec));
  rec->arity = arity;
  rec->func = func;
  ret->value = rec;
  return ret;
}
