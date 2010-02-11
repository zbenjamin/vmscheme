#ifndef VMSCHEME_EVAL_H
#define VMSCHEME_EVAL_H

#include <instruction.h>
#include <stack.h>
#include <environment.h>

void eval(struct instruction *prog, struct stack *stk,
          struct environment *env);

#endif // VMSCHEME_EVAL_H
