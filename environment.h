#ifndef VMSCHEME_ENVIRONMENT_H
#define VMSCHEME_ENVIRONMENT_H

struct environment {
  char** names;
  struct object** values;
  unsigned int size;
};

void env_define(struct environment *env, char *name,
                struct object *val);
struct object* env_lookup(struct environment *env, char *name);

#endif // VMSCHEME_ENVIRONMENT_H
