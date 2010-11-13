#ifndef VMSCHEME_EVAL_H
#define VMSCHEME_EVAL_H

#include <instruction.h>
#include <stack.h>
#include <environment.h>
#include <object.h>

struct vm_context {
  struct object obj;
  struct codeptr *pc;
  struct stack *stk;
  struct environment *env;
};

struct object *eval(struct pair *form,
                    struct environment *env);
struct object *eval_sequence(struct pair *forms,
                             struct environment *env);
struct object *apply(struct procedure *func, struct pair *args,
                     struct vm_context **ctx);
struct object *apply_wrap(struct object *func, struct object *args,
                          struct vm_context **ctx);
struct object *apply_and_run(struct procedure *func,
                             struct pair *args,
                             struct vm_context **ctx);

#endif // VMSCHEME_EVAL_H
