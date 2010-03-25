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
  for (i = 0; i < env->eval->size; ++i) {
    free(env->eval->names[i]);
    DEC_REF(env->eval->values[i]);
  }
  free(env->eval->names);
  free(env->eval->values);
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
    DEC_REF(env->eval->values[idx]);
    env->eval->values[idx] = val;
    return;
  }

  char **names;
  struct object **values;
  names = malloc(sizeof(char*) * (env->eval->size + 1));
  values = malloc(sizeof(struct object*) * (env->eval->size + 1));
  memcpy(names, env->eval->names,
         sizeof(char*) * env->eval->size);
  memcpy(values, env->eval->values,
         sizeof(struct object*) * env->eval->size);
  names[env->eval->size] = strdup(name);
  values[env->eval->size] = val;
  free(env->eval->names);
  free(env->eval->values);

  env->eval->names = names;
  env->eval->values = values;
  env->eval->size++;
}

struct object*
env_lookup(struct object *env, const char *name)
{
  while (env != NULL) {
    int idx = env_find_idx(env, name);
    if (idx != -1) {
      return env->eval->values[idx];
    }
    env = env->eval->parent;
  }

  return NULL;
}    

int
env_find_idx(struct object *env, const char *name)
{
  int i;
  for (i = 0; i < env->eval->size; ++i) {
    if (strcmp(env->eval->names[i], name) == 0) {
      return i;
    }
  }

  return -1;
}

void
env_bind_names(struct object *env, const struct object *names,
               const struct object *values)
{
  while (names != NIL) {
    env_define(env, car(names)->sval, car(values));
    names = cdr(names);
    values = cdr(values);
  }
}

