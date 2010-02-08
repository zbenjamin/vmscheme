#ifndef VMSCHEME_OBJECT_H
#define VMSCHEME_OBJECT_H

#include <type.h>

struct object {
  struct type *type;
  union {
    void* value;
    int ival;
    const char* sval;
  };
  int refcount;  // currently unused
};

#define INC_REF(x) ++((x)->refcount)
#define DEC_REF(x)                              \
  do {                                          \
    struct object *__obj = (x);                 \
    --(__obj->refcount);                        \
    if (__obj->refcount == 0) {                 \
      obj_dealloc(__obj);                       \
    }                                           \
  } while (0)


struct proc_rec {
  const struct object *params;
  const struct object *body;
  struct environment *env;
};

struct primitive_proc_rec {
  unsigned int arity;
  void *func;
};

extern struct object *NIL;

void init_nil();

struct object* make_integer(int x);
struct object* make_string(const char *str);
struct object* make_symbol(const char *str);
struct object* make_pair(struct object *car,
                         struct object *cdr);
struct object* make_procedure(const struct object *params,
                              const struct object *body,
                              struct environment *env);
struct object* make_primitive_procedure(void *func,
                                        unsigned int arity);

void print_obj(struct object *obj);

#endif // VMSCHEME_OBJECT_H