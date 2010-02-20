#include <eval.h>

#include <object.h>
#include <opcode.h>
#include <primitive_procedures.h>

#include <stdio.h>
#include <stdlib.h>

static void eval_instruction(struct instruction ins,
                             struct stack *stk,
                             struct environment *env);
static void eval_compound_call(struct stack *stk, struct object *func,
                               struct object *args);
static void eval_call(struct stack *stk, struct environment *env);
static void eval_call1(struct stack *stk, struct object *func,
                       struct object *args);
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
    value = env_lookup(env, ins.arg->sval);
    if (! value) {
      printf("Unbound name: %s\n", ins.arg->sval);
      exit(1);
    }
    push_stack(stk, value);
    break;
  case CALL:
    printf("CALL instruction\n");
    eval_call(stk, env);
    break;
  case DEFINE:
    printf("DEFINE instruction\n");
    value = pop_stack(stk);
    env_define(env, ins.arg->sval, value);
    break;
  case LAMBDA:
    printf("LAMBDA instruction\n");
    struct object *templ = ins.arg;
    push_stack(stk, make_procedure(templ->proc_val->params,
                                   templ->proc_val->code,
                                   env));
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

  int num = num_args->ival;
  while (num) {
    args = make_pair(pop_stack(stk), args);
    --num;
  }

  struct object *func = pop_stack(stk);
  if (func->type->code != PRIMITIVE_PROC_TYPE
      && func->type->code != PROCEDURE_TYPE) {
    printf("Cannot apply object of type %s\n",
           func->type->name);
    exit(1);
  }

  unsigned int num_params =
    (func->type->code == PROCEDURE_TYPE
     ? list_length_int(func->proc_val->params)
     : func->pproc_val->arity);

  if (num_args->ival != num_params) {
      printf("Incorrect number of arguments to ");
      print_obj(func);
      printf("\n");
      exit(1);
  }

  if (func->type->code == PROCEDURE_TYPE) {
    eval_compound_call(stk, func, args);
    return;
  }

  // primitive function dispatch
  switch (num_args->ival) {
  case 1:
    eval_call1(stk, func, args);
    break;
  case 2:
    eval_call2(stk, func, args);
    break;
  default:
    printf("primitive procedures with %d arguments "
           "not supported yet\n",
           num_args->ival);
    exit(1);
  }
}

void
eval_compound_call(struct stack *stk, struct object *func,
                   struct object *args) {
  struct environment *new_env = make_environment(func->proc_val->env);
  env_bind_names(new_env, func->proc_val->params, args);
  eval(func->proc_val->code->cval, stk, new_env);
}

void
eval_call1(struct stack *stk, struct object *func,
           struct object *args)
{
  struct object *arg1 = car(args);

  struct object *result;
  struct primitive_proc_rec *rec = func->pproc_val;
  struct object *(*function)(struct object*);
  function = (struct object *(*)(struct object*)) rec->func;
  result = function(arg1);
  push_stack(stk, result);
}

void
eval_call2(struct stack *stk, struct object *func,
           struct object *args)
{
  struct object *arg1 = car(args);
  args = cdr(args);
  struct object *arg2 = car(args);
  args = cdr(args);

  struct object *result;
  struct primitive_proc_rec *rec = func->pproc_val;
  struct object *(*function)(struct object*,
                             struct object*);
  function = (struct object *(*)(struct object*,
                                 struct object*)) rec->func;
  result = function(arg1, arg2);
  push_stack(stk, result);
}
