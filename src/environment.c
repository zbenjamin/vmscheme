#include <environment.h>

#include <object.h>
#include <primitive_procedures.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int env_find_idx(struct environment *env, const char *name);

struct environment *global_env;

void
init_global_env(void)
{
  global_env = make_environment(NULL);
  global_env->obj.refcount = -1;
}

void
dealloc_env(struct environment *env)
{
  int i;
  int size = array_size(&env->values);
  for (i = 0; i < size; ++i) {
    free(*str_array_ref(&env->names, i));
    DEC_REF(*obj_array_ref(&env->values, i));
  }
  array_dealloc(&env->names);
  array_dealloc(&env->values);
  DEC_REF(&env->parent->obj);
  free(env);
}

void
env_define(struct environment *env, const char *name,
           struct object *val)
{
  INC_REF(val);
  int idx = env_find_idx(env, name);
  if (idx != -1) {
    DEC_REF(*obj_array_ref(&env->values, idx));
    array_set(&env->values, idx, &val);
    return;
  }

  char *dupname = strdup(name);
  array_add(&env->names, &dupname);
  array_add(&env->values, &val);
}

void
env_set(struct environment *env, const char *name, struct object *val)
{
  int idx;
  do {
    idx = env_find_idx(env, name);
    if (idx != -1) {
      DEC_REF(*obj_array_ref(&env->values, idx));
      array_set(&env->values, idx, &val);
      INC_REF(val);
      return;
    }
    env = env->parent;
  } while (env);

  printf("Unbound name: %s\n", name);
  exit(1);
}

struct object*
env_lookup(struct environment *env, const char *name)
{
  while (env != NULL) {
    int idx = env_find_idx(env, name);
    if (idx != -1) {
      return *obj_array_ref(&env->values, idx);
    }
    env = env->parent;
  }

  return NULL;
}    

int
env_find_idx(struct environment *env, const char *name)
{
  int i;
  for (i = 0; i < array_size(&env->names); ++i) {
    if (strcmp(*str_array_ref(&env->names, i), name) == 0) {
      return i;
    }
  }

  return -1;
}

void
env_bind_names(struct environment *env, const struct object *names,
               struct pair *values)
{
  while (names != &NIL->obj) {
    assert(names->type->code == SYMBOL_TYPE || values != NIL);
    if (names->type->code == SYMBOL_TYPE) {
      env_define(env, container_of(names, struct symbol, obj)->value,
                 &values->obj);
      return;
    }
    assert(names->type->code == PAIR_TYPE);
    struct pair *pnames = container_of(names, struct pair, obj);
    assert(pnames->car->type->code == SYMBOL_TYPE);
    env_define(env,
               container_of(pnames->car, struct symbol, obj)->value,
               values->car);
    names = pnames->cdr;
    assert(values->cdr->type->code == PAIR_TYPE
           || values->cdr->type->code == NIL_TYPE);
    values = container_of(values->cdr, struct pair, obj);
  }
}

