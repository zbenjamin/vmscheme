#ifndef VMSCHEME_EVAL_H
#define VMSCHEME_EVAL_H

#include <instruction.h>
#include <stack.h>
#include <environment.h>

struct vm_context {
  struct instruction *pc;
  struct stack *stk;
  struct object *env;
};

struct object *eval(struct object *form,
                    struct object *env);
struct object *eval_sequence(struct object *forms,
                             struct object *env);
struct object *apply(struct object *func, struct object *args,
                     struct vm_context *ctx);
struct object *apply_and_run(struct object *func, struct object *args,
                             struct vm_context *ctx);

#endif // VMSCHEME_EVAL_H
