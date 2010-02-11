#include <eval.h>

#include <object.h>
#include <opcode.h>
#include <primitive_procedures.h>

#include <stdio.h>
#include <stdlib.h>

static void eval_instruction(struct instruction ins,
                             struct stack *stk,
                             struct environment *env);
static void eval_call(struct stack *stk, struct environment *env);
static void eval_call2(struct stack *stk, struct object *func,
                       struct object *args);

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
      printf("Unbound name: %s\n", (char*) ins.arg);
      exit(1);
    }
    push_stack(stk, value);
    break;
  case CALL:
    printf("CALL instruction\n");
    eval_call(stk, env);
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

void
eval_call(struct stack *stk, struct environment *env)
{
  struct object *num_args = pop_stack(stk);
  struct object *args = NIL;
  if (num_args->type->code != INTEGER_TYPE) {
    printf("Internal error: number of arguments to call "
           "is not an integer\n");
    exit(1);
  }

  struct object *func = pop_stack(stk);

  int num = num_args->ival;
  while (num) {
    args = make_pair(pop_stack(stk), args);
    --num;
  }

  // only primitive functions for now
  switch (num_args->ival) {
  case 2:
    eval_call2(stk, func, args);
    break;
  default:
    printf("only 2 arguments supported right now\n");
    exit(1);
  }
}

void
eval_call2(struct stack *stk, struct object *func,
           struct object *args)
{
  struct object *arg2 = car(args);
  args = cdr(args);
  struct object *arg1 = car(args);
  args = cdr(args);

  struct object *result;
  struct primitive_proc_rec *rec;
  rec = (struct primitive_proc_rec*) func->value;
  struct object *(*function)(struct object*,
                             struct object*);
  function = (struct object *(*)(struct object*,
                                 struct object*)) rec->func;
  result = function(arg1, arg2);
  push_stack(stk, result);
}