#include <stdio.h>
#include <stdlib.h>

#include <opcode.h>
#include <environment.h>
#include <instruction.h>
#include <stack.h>
#include <type.h>

void
eval_instruction(struct instruction ins, struct stack *stk,
                 struct environment *env)
{
  int num_args;
  struct object *value;

  switch (ins.op) {
  case NONE:
    printf("Error: tried to execute a NONE op\n");
    exit(1);
    break;
  case PUSH:
    push_stack(stk, ins.arg);
    break;
  case LOOKUP:
    push_stack(stk, env_lookup(env, (char*) ins.arg));
    break;
  case CALL:
    num_args = pop_stack(stk);
    // pull off args
    // call procedure
    break;
  case DEFINE:
    value = pop_stack(stk);
    env_define(env, (char*) ins.arg, value);
    break;
  default:
    printf("Error: unknown opcode\n");
    exit(1);
  }
}

int
main(int argc, char* argv[])
{
  init_builtin_types();
  return 0;
}
