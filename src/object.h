#ifndef VMSCHEME_OBJECT_H
#define VMSCHEME_OBJECT_H

#include <type.h>
#include <debug.h>
#include <stddef.h>

#include <assert.h>

#define container_of(ptr, type, member) ({                   \
      const typeof(((type *)0)->member) *__mptr = (ptr);     \
      (type*) ((char*) __mptr - offsetof(type,member));})

struct object {
  struct type *type;
  int refcount;
  struct debuginfo *dinfo;
};

struct integer {
  struct object obj;
  int value;
};

struct string {
  struct object obj;
  char* value;
};

struct symbol {
  struct object obj;
  char* value;
};

struct pair {
  struct object obj;
  struct object *car;
  struct object *cdr;
};

enum procedure_type {
  PRIMITIVE,
  COMPOUND
};

struct procedure {
  struct object obj;
  unsigned int type;
};

struct compound_proc {
  struct procedure proc;
  // could be a list or a symbol
  struct object *params;
  struct code *code;
  struct environment *env;
};

struct prim_proc {
  struct procedure proc;
  unsigned int arity;
  void *func;
  const char *name;
  unsigned int takes_ctx;
};

struct instruction;

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

extern struct pair *NIL;
extern struct object *UNSPECIFIC;
extern struct object *TRUE;
extern struct object *FALSE;

void init_singleton_objects(void);

struct integer* make_integer(int x);
struct string* make_string(char *str);
struct symbol* make_symbol(char *str);
struct pair* make_pair(struct object *car,
                       struct object *cdr);
struct compound_proc*
make_compound_procedure(struct object *params,
                        struct code *code,
                        struct environment *env);
struct prim_proc* make_primitive_procedure(void *func,
                                           unsigned int arity,
                                           const char* name,
                                           unsigned int takes_ctx);
struct code* make_code(struct instruction *stream);
struct codeptr* make_codeptr(struct code *base, size_t offset);
struct environment* make_environment(struct environment *parent);

struct object* print_obj(struct object *obj);

#endif // VMSCHEME_OBJECT_H
