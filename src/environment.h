#ifndef VMSCHEME_ENVIRONMENT_H
#define VMSCHEME_ENVIRONMENT_H

struct environment {
  char **names;
  struct object **values;
  unsigned int size;
  struct object *parent;
};

void init_global_env();

void dealloc_env(struct object *env);
void env_define(struct object *env, const char *name,
                struct object *val);
struct object* env_lookup(struct object *env, const char *name);
void env_bind_names(struct object *env,
                    const struct object *names,
                    const struct object *values);

extern struct object *global_env;

#endif // VMSCHEME_ENVIRONMENT_H
