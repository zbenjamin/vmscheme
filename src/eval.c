#include <eval.h>

#include <compiler.h>
#include <object.h>
#include <opcode.h>
#include <primitive_procedures.h>
#include <utils.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void eval_instructions(struct instruction *prog,
                              struct stack *stk,
                              struct object *env);
static int eval_instruction(struct instruction **ins,
                             struct stack *stk,
                             struct object **env);
static void eval_call(struct instruction **pc, struct stack *stk,
                      struct object **env);
static void eval_call0(struct stack *stk, struct object *func,
                       struct object *args);
static void eval_call1(struct stack *stk, struct object *func,
                       struct object *args);
static void eval_call2(struct stack *stk, struct object *func,
                       struct object *args);
static void eval_if(struct instruction **pc, struct stack *stk,
                    struct object **env);


struct object *
eval(struct object *form, struct object *env)
{
  struct object *seq = make_pair(form, NIL);
  struct object *ret = eval_sequence(seq, env);
  DEC_REF(seq);
  return ret;
}

struct object *
eval_sequence(struct object *forms, struct object *env)
{
  struct instruction *prog = compile(forms);
  struct stack *stk = make_stack(1024);
  // push magic "end of instructions" return address
  struct object *end_marker = make_code(NULL);
  end_marker->refcount = -1;
  push_stack(stk, end_marker);
  push_stack(stk, env);

  eval_instructions(prog, stk, env);
  struct object *value = pop_stack(stk);
  dealloc_bytecode(prog);
  assert(stack_empty(stk));
  dealloc_stack(stk);
  return value;
}

void
eval_instructions(struct instruction *prog, struct stack *stk,
                  struct object *env)
{
  struct instruction *pc = prog;

  while (! eval_instruction(&pc, stk, &env)) { }
}

int
eval_instruction(struct instruction **pc, struct stack *stk,
                 struct object **env)
{
  struct object *value;

  switch ((*pc)->op) {
  case NONE:
    printf("Error: tried to execute a NONE op\n");
    exit(1);
    break;
  case PUSH:
    /* printf("PUSH instruction\n"); */
    push_stack(stk, (*pc)->arg);
    (*pc)++;
    break;
  case POP:
    /* printf("POP instruction\n"); */
    value = pop_stack(stk);
    DEC_REF(value);
    (*pc)++;
    break;
  case LOOKUP:
    /* printf("LOOKUP instruction\n"); */
    value = env_lookup(*env, (*pc)->arg->sval);
    if (! value) {
      printf("Unbound name: %s\n", (*pc)->arg->sval);
      exit(1);
    }
    push_stack(stk, value);
    (*pc)++;
    break;
  case CALL:
    /* printf("CALL instruction @ %p\n", *pc); */
    eval_call(pc, stk, env);
    break;
  case RET:
    value = pop_stack(stk);
    struct object *orig_env = pop_stack(stk);
    struct object *retaddr = pop_stack(stk);
    /* printf("RET instruction @ %p to %p\n", *pc, retaddr->cval); */
    push_stack(stk, value);
    *env = orig_env;
    *pc = retaddr->cval;
    if (*pc == NULL) {
      return 1;
    }
    break;
  case DEFINE:
    /* printf("DEFINE instruction\n"); */
    value = pop_stack(stk);
    env_define(*env, (*pc)->arg->sval, value);
    (*pc)++;
    break;
  case LAMBDA:
    /* printf("LAMBDA instruction\n"); */
    value = (*pc)->arg;
    push_stack(stk, make_procedure(value->proc_val->params,
                                   value->proc_val->code,
                                   *env));
    (*pc)++;
    break;
  case IF:
    /* printf("IF instruction\n"); */
    eval_if(pc, stk, env);
    break;
  default:
    printf("Error: unknown opcode: %d\n", (*pc)->op);
    exit(1);
  }

  return 0;
}

void
eval_call(struct instruction **pc, struct stack *stk, 
          struct object **env)
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
    // push the return value onto the stack.  We don't want the
    // return addr garbage collected.
    struct object *retaddr = make_code(*pc + 1);
    retaddr->refcount = -1;

    push_stack(stk, retaddr);
    push_stack(stk, *env);

    struct object *new_env;
    new_env = make_environment(func->proc_val->env);
    env_bind_names(new_env, func->proc_val->params, args);
    *pc = func->proc_val->code->cval;
    *env = new_env;
    return;
  }

  // primitive function dispatch
  switch (num_args->ival) {
  case 0:
    eval_call0(stk, func, args);
    break;
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
  ++(*pc);
}

void
eval_call0(struct stack *stk, struct object *func,
           struct object *args)
{
  struct object *result;
  struct primitive_proc_rec *rec = func->pproc_val;
  struct object *(*function)();
  function = (struct object *(*)()) rec->func;
  result = function();
  push_stack(stk, result);
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

void
eval_if(struct instruction **pc, struct stack *stk,
        struct object **env)
{
  struct object *testval = pop_stack(stk);
  struct object *alt = pop_stack(stk);
  struct object *conseq = pop_stack(stk);

  struct object *action;
  if (testval == FALSE) {
    action = alt;
  } else {
    action = conseq;
  }

  push_stack(stk, action);
  push_stack(stk, make_integer(0));
  eval_call(pc, stk, env);
}
