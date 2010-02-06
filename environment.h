#ifndef VMSCHEME_ENVIRONMENT_H
#define VMSCHEME_ENVIRONMENT_H

struct environment {
  char** names;
  struct object** values;
  unsigned int size;
  struct environment *parent;
};

struct environment* make_environment(struct environment *parent);
void env_define(struct environment *env, char *name,
                struct object *val);
struct object* env_lookup(struct environment *env, char *name);

void init_global_env();

struct environment *global_env;

#endif // VMSCHEME_ENVIRONMENT_H
