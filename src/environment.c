#include <environment.h>

#include <object.h>
#include <primitive_procedures.h>

#include <stdlib.h>
#include <string.h>

static int env_find_idx(struct object *env, const char *name);

struct object *global_env;

void
init_global_env()
{
  global_env = make_environment(NULL);
  global_env->refcount = -1;
}

void
dealloc_env(struct object *env)
{
  int i;
  int size = array_size(&env->eval->values);
  for (i = 0; i < size; ++i) {
    DEC_REF(array_ref(&env->eval->values, i));
  }
  array_dealloc(&env->eval->names, 1);
  array_dealloc(&env->eval->values, 0);
  DEC_REF(env->eval->parent);
  free(env->eval);
}

void
env_define(struct object *env, const char *name,
           struct object *val)
{
  INC_REF(val);
  int idx = env_find_idx(env, name);
  if (idx != -1) {
    DEC_REF(array_ref(&env->eval->values, idx));
    array_set(&env->eval->values, idx, val);
    return;
  }

  array_add(&env->eval->names, strdup(name));
  array_add(&env->eval->values, val);
}

struct object*
env_lookup(struct object *env, const char *name)
{
  while (env != NULL) {
    int idx = env_find_idx(env, name);
    if (idx != -1) {
      return array_ref(&env->eval->values, idx);
    }
    env = env->eval->parent;
  }

  return NULL;
}    

int
env_find_idx(struct object *env, const char *name)
{
  int i;
  for (i = 0; i < array_size(&env->eval->names); ++i) {
    if (strcmp(array_ref(&env->eval->names, i), name) == 0) {
      return i;
    }
  }

  return -1;
}

void
env_bind_names(struct object *env, const struct object *names,
               struct object *values)
{
  while (names != NIL) {
    if (names->type->code == SYMBOL_TYPE) {
      env_define(env, names->sval, values);
      return;
    }
    env_define(env, car(names)->sval, car(values));
    names = cdr(names);
    values = cdr(values);
  }
}

