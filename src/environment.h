#ifndef VMSCHEME_ENVIRONMENT_H
#define VMSCHEME_ENVIRONMENT_H

#include <array.h>
#include <object.h>

struct environment {
  struct object obj;
  struct array names;
  struct array values;
  struct environment *parent;
};

void init_global_env(void);

void dealloc_env(struct environment *env);
void env_define(struct environment *env, const char *name,
                struct object *val);
void env_set(struct environment *env, const char *name,
             struct object *val);
struct object* env_lookup(struct environment *env, const char *name);
void env_bind_names(struct environment *env,
                    const struct object *names,
                    struct pair *values);

extern struct environment *global_env;

#endif // VMSCHEME_ENVIRONMENT_H
