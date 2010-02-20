#ifndef VMSCHEME_ENVIRONMENT_H
#define VMSCHEME_ENVIRONMENT_H

struct environment {
  const char **names;
  struct object **values;
  unsigned int size;
  struct environment *parent;
};

void init_global_env();

struct environment* make_environment(struct environment *parent);
void env_define(struct environment *env, const char *name,
                struct object *val);
struct object* env_lookup(struct environment *env, const char *name);
void env_bind_names(struct environment *env,
                    const struct object *names,
                    const struct object *values);

struct environment *global_env;

#endif // VMSCHEME_ENVIRONMENT_H
