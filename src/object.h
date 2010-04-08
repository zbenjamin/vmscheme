#ifndef VMSCHEME_OBJECT_H
#define VMSCHEME_OBJECT_H

#include <type.h>
#include <instruction.h>
#include <debug.h>

#include <assert.h>

struct object {
  struct type *type;
  union {
    int ival;
    char *sval;
    struct object **pval;
    struct proc_rec *proc_val;
    struct primitive_proc_rec *pproc_val;
    struct instruction *cval;
    struct environment *eval;
  };
  int refcount;
  struct debuginfo *dinfo;
};

struct proc_rec {
  struct object *params;
  struct object *code;
  struct object *env;
};

struct primitive_proc_rec {
  unsigned int arity;
  void *func;
  const char *name;
  unsigned int takes_ctx;
};

#define INC_REF(x)                                  \
  do {                                              \
    struct object *__obj = (x);                     \
    if (__obj->refcount >= 0) {                     \
      ++(__obj->refcount);                          \
    }                                               \
  } while (0)

#define DEC_REF(x)                              \
  do {                                          \
    struct object *__obj = (x);                 \
    assert(__obj->refcount != 0);               \
    if (__obj->refcount > 0) {                  \
      --(__obj->refcount);                      \
    }                                           \
    _MAYBE_DEALLOC_OBJ(__obj);                  \
  } while (0)

#define _MAYBE_DEALLOC_OBJ(x)                   \
  do {                                          \
    struct object *___obj = (x);                \
    if (___obj->refcount == 0) {                \
      dealloc_obj(___obj);                      \
    }                                           \
  } while (0)

#define YIELD_OBJ(x) _MAYBE_DEALLOC_OBJ(x)

void dealloc_obj(struct object *obj);

extern struct object *NIL;
extern struct object *TRUE;
extern struct object *FALSE;

void init_singleton_objects(void);

struct object* make_integer(int x);
struct object* make_string(char *str);
struct object* make_symbol(char *str);
struct object* make_pair(struct object *car,
                         struct object *cdr);
struct object* make_procedure(struct object *params,
                              struct object *code,
                              struct object *env);
struct object* make_primitive_procedure(void *func,
                                        unsigned int arity,
                                        const char* name,
                                        unsigned int takes_ctx);
struct object* make_code(struct instruction *code);
struct object* make_environment(struct object *parent);

struct object* print_obj(struct object *obj);

#endif // VMSCHEME_OBJECT_H
