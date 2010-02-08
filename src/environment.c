#include <environment.h>

#include <object.h>

#include <stdlib.h>
#include <string.h>

static int env_find_idx(struct environment *env, char *name);

void
init_global_env()
{
  global_env = make_environment(NULL);
}

struct environment*
make_environment(struct environment *parent)
{
  struct environment *ret = malloc(sizeof(struct environment));
  ret->names = 0;
  ret->values = 0;
  ret->size = 0;
  ret->parent = parent;

  return ret;
}

void
env_define(struct environment *env, char *name, struct object *val)
{
  int idx = env_find_idx(env, name);
  if (idx != -1) {
    env->values[idx] = val;
    return;
  }

  char** names;
  struct object** values;
  names = malloc(sizeof(char*) * (env->size + 1));
  values = malloc(sizeof(struct object*) * (env->size + 1));
  memcpy(names, env->names, sizeof(char*) * env->size);
  memcpy(values, env->values, sizeof(struct object*) * env->size);
  names[env->size] = name;
  values[env->size] = val;
  free(env->names);
  free(env->values);

  env->names = names;
  env->values = values;
  env->size++;
}

struct object*
env_lookup(struct environment *env, char *name)
{
  int idx = env_find_idx(env, name);
  if (idx == -1) {
    return NULL;
  }

  return env->values[idx];
}    

int
env_find_idx(struct environment *env, char *name)
{
  int i;
  for (i = 0; i < env->size; ++i) {
    if (strcmp(env->names[i], name) == 0) {
      return i;
    }
  }

  return -1;
}