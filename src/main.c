#include <stdio.h>
#include <stdlib.h>

#include <opcode.h>
#include <compiler.h>
#include <environment.h>
#include <instruction.h>
#include <parser.h>
#include <primitive_procedures.h>
#include <stack.h>
#include <type.h>

static void eval_instruction(struct instruction ins,
                             struct stack *stk,
                             struct environment *env);

void
eval(struct instruction *prog, struct stack *stk,
     struct environment *env)
{
  struct instruction *pc;
  for (pc = prog; pc->op != END; ++pc) {
    eval_instruction(*pc, stk, env);
  }
}

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
    printf("PUSH instruction\n");
    push_stack(stk, ins.arg);
    break;
  case LOOKUP:
    printf("LOOKUP instruction\n");
    value = env_lookup(env, (char*) ins.arg);
    if (! value) {
      printf("Unbound name: %s\n", ins.arg);
      exit(1);
    }
    push_stack(stk, value);
    break;
  case CALL:
    printf("CALL instruction\n");
    /* num_args = pop_stack(stk); */
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
  init_global_env();
  init_builtin_types();
  init_nil();
  init_primitive_procs();

  int ret;
  char *buf = NULL;
  size_t nchars;
  while (1) {
    printf("> ");
    ret = getline(&buf, &nchars, stdin);
    if (ret == -1) {
      break;
    }
    struct object *input = parse(buf);
    print_obj(input);
    printf("\n");

    struct instruction *prog = compile(input);
    struct stack *stk = make_stack(1024);
    eval(prog, stk, global_env);
    struct object *value = pop_stack(stk);
    print_obj(value);
    printf("\n");

    free(buf);
    free(stk);
    free(prog);
    buf = NULL;
  }
  return 0;
}
