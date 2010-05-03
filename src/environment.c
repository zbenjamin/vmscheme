#include <environment.h>

#include <object.h>
#include <primitive_procedures.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int env_find_idx(struct object *env, const char *name);

struct object *global_env;

void
init_global_env(void)
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
    free(*str_array_ref(&env->eval->names, i));
    DEC_REF(*obj_array_ref(&env->eval->values, i));
  }
  array_dealloc(&env->eval->names);
  array_dealloc(&env->eval->values);
  DEC_REF(env->eval->parent);
  free(env->eval);
}

void
env_define(struct object *env, const char *name, struct object *val)
{
  INC_REF(val);
  int idx = env_find_idx(env, name);
  if (idx != -1) {
    DEC_REF(*obj_array_ref(&env->eval->values, idx));
    array_set(&env->eval->values, idx, &val);
    return;
  }

  char *dupname = strdup(name);
  array_add(&env->eval->names, &dupname);
  array_add(&env->eval->values, &val);
}

void
env_set(struct object *env, const char *name, struct object *val)
{
  int idx;
  do {
    idx = env_find_idx(env, name);
    if (idx != -1) {
      DEC_REF(*obj_array_ref(&env->eval->values, idx));
      array_set(&env->eval->values, idx, &val);
      INC_REF(val);
      return;
    }
    env = env->eval->parent;
  } while (env);

  printf("Unbound name: %s\n", name);
  exit(1);
}

struct object*
env_lookup(struct object *env, const char *name)
{
  while (env != NULL) {
    int idx = env_find_idx(env, name);
    if (idx != -1) {
      return *obj_array_ref(&env->eval->values, idx);
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
    if (strcmp(*str_array_ref(&env->eval->names, i), name) == 0) {
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

