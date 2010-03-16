#include <eval.h>

#include <compiler.h>
#include <object.h>
#include <opcode.h>
#include <primitive_procedures.h>
#include <utils.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void eval_instructions(struct vm_context *ctx);
static int eval_instruction(struct vm_context *ctx);
static void eval_call(struct vm_context *ctx);
static void eval_if(struct vm_context *ctx);
static struct object *primitive_apply0(struct object *func,
                                       struct object *args,
                                       struct vm_context *ctx);
static struct object *primitive_apply1(struct object *func,
                                       struct object *args,
                                       struct vm_context *ctx);
static struct object *primitive_apply2(struct object *func,
                                       struct object *args,
                                       struct vm_context *ctx);


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
  struct vm_context ctx = { prog, stk, env };

  eval_instructions(&ctx);
  struct object *value = pop_stack(stk);
  dealloc_bytecode(prog);
  assert(stack_empty(stk));
  dealloc_stack(stk);
  return value;
}

void
eval_instructions(struct vm_context *ctx)
{
  while (! eval_instruction(ctx)) { }
}

int
eval_instruction(struct vm_context *ctx)
{
  struct object *value;

  switch (ctx->pc->op) {
  case NONE:
    printf("Error: tried to execute a NONE op\n");
    exit(1);
    break;
  case PUSH:
    /* printf("PUSH instruction\n"); */
    push_stack(ctx->stk, ctx->pc->arg);
    ++ctx->pc;
    break;
  case POP:
    /* printf("POP instruction\n"); */
    value = pop_stack(ctx->stk);
    DEC_REF(value);
    ++ctx->pc;
    break;
  case LOOKUP:
    /* printf("LOOKUP instruction\n"); */
    value = env_lookup(ctx->env, ctx->pc->arg->sval);
    if (! value) {
      printf("Unbound name: %s\n", ctx->pc->arg->sval);
      exit(1);
    }
    push_stack(ctx->stk, value);
    ++ctx->pc;
    break;
  case CALL:
    /* printf("CALL instruction @ %p\n", *pc); */
    eval_call(ctx);
    break;
  case RET:
    value = pop_stack(ctx->stk);
    struct object *orig_env = pop_stack(ctx->stk);
    struct object *retaddr = pop_stack(ctx->stk);
    /* printf("RET instruction @ %p to %p\n", *pc, retaddr->cval); */
    push_stack(ctx->stk, value);
    ctx->env = orig_env;
    ctx->pc = retaddr->cval;
    if (ctx->pc == NULL) {
      return 1;
    }
    break;
  case DEFINE:
    /* printf("DEFINE instruction\n"); */
    value = pop_stack(ctx->stk);
    env_define(ctx->env, ctx->pc->arg->sval, value);
    ++ctx->pc;
    break;
  case LAMBDA:
    /* printf("LAMBDA instruction\n"); */
    value = ctx->pc->arg;
    push_stack(ctx->stk, make_procedure(value->proc_val->params,
                                        value->proc_val->code,
                                        ctx->env));
    ++ctx->pc;
    break;
  case IF:
    /* printf("IF instruction\n"); */
    eval_if(ctx);
    break;
  default:
    printf("Error: unknown opcode: %d\n", ctx->pc->op);
    exit(1);
  }

  return 0;
}

void
eval_call(struct vm_context *ctx)
{
  struct object *num_args = pop_stack(ctx->stk);
  struct object *args = NIL;
  if (num_args->type->code != INTEGER_TYPE) {
    printf("Internal error: number of arguments to call "
           "is not an integer\n");
    exit(1);
  }

  int num = num_args->ival;
  while (num) {
    args = make_pair(pop_stack(ctx->stk), args);
    --num;
  }

  struct object *func = pop_stack(ctx->stk);
  struct object *result;
  result = apply(func, args, ctx);
  // we get a result back for primitive functions, but compound
  // functions muck with the vm context instead
  if (result != NULL) {
    ++ctx->pc;
    push_stack(ctx->stk, result);
  }
}

void
eval_if(struct vm_context *ctx)
{
  struct object *testval = pop_stack(ctx->stk);
  struct object *alt = pop_stack(ctx->stk);
  struct object *conseq = pop_stack(ctx->stk);

  struct object *action;
  if (testval == FALSE) {
    action = alt;
  } else {
    action = conseq;
  }

  push_stack(ctx->stk, action);
  push_stack(ctx->stk, make_integer(0));
  eval_call(ctx);
}

struct object *
apply(struct object *func, struct object *args,
      struct vm_context *ctx)
{
  unsigned int num_args = list_length_int(args);

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

  if (num_args != num_params) {
      printf("Incorrect number of arguments to ");
      print_obj(func);
      printf("\n");
      exit(1);
  }

  if (func->type->code == PROCEDURE_TYPE) {
    // Push the return value onto the stack.  We don't want the
    // return addr garbage collected.  Also account for the case that
    // we were called from apply_and_run (and thus the pc might be
    // zero).
    struct object *retaddr;
    if (ctx->pc) {
      retaddr = make_code(ctx->pc + 1);
      retaddr->refcount = -1;
    } else {
      retaddr = make_code(NULL);
      retaddr->refcount = -1;
    }

    push_stack(ctx->stk, retaddr);
    push_stack(ctx->stk, ctx->env);

    struct object *new_env;
    new_env = make_environment(func->proc_val->env);
    env_bind_names(new_env, func->proc_val->params, args);
    ctx->pc = func->proc_val->code->cval;
    ctx->env = new_env;
    return NULL;
  }

  struct object *result;
  // primitive function dispatch
  switch (num_args) {
  case 0:
    result = primitive_apply0(func, args, ctx);
    break;
  case 1:
    result = primitive_apply1(func, args, ctx);
    break;
  case 2:
    result = primitive_apply2(func, args, ctx);
    break;
  default:
    printf("primitive procedures with %d arguments "
           "not supported yet\n",
           num_args);
    exit(1);
  }
  return result;
}

struct object *
primitive_apply0(struct object *func, struct object *args,
                 struct vm_context *ctx)
{
  struct primitive_proc_rec *rec = func->pproc_val;
  if (rec->takes_ctx) {
    struct object *(*function)(struct vm_context*);
    function = (struct object *(*)(struct vm_context*)) rec->func;
    return function(ctx);
  } else {
    struct object *(*function)();
    function = (struct object *(*)()) rec->func;
    return function();
  }
}

struct object *
primitive_apply1(struct object *func, struct object *args,
                 struct vm_context *ctx)
{
  struct object *arg1 = car(args);

  struct primitive_proc_rec *rec = func->pproc_val;
  if (rec->takes_ctx) {
    struct object *(*function)(struct object*, struct vm_context*);
    function = (struct object *(*)(struct object*,
                                   struct vm_context*)) rec->func;
    return function(arg1, ctx);
  } else {
    struct object *(*function)(struct object*);
    function = (struct object *(*)(struct object*)) rec->func;
    return function(arg1);
  }
}

struct object *
primitive_apply2(struct object *func, struct object *args,
                 struct vm_context *ctx)
{
  struct object *arg1 = car(args);
  args = cdr(args);
  struct object *arg2 = car(args);
  args = cdr(args);

  struct primitive_proc_rec *rec = func->pproc_val;
  if (rec->takes_ctx) {
    struct object *(*function)(struct object*,
                               struct object*,
                               struct vm_context*);
    function = (struct object *(*)(struct object*,
                                   struct object*,
                                   struct vm_context*)) rec->func;
    return function(arg1, arg2, ctx);
  } else {
    struct object *(*function)(struct object*,
                               struct object*);
    function = (struct object *(*)(struct object*,
                                   struct object*)) rec->func;
    return function(arg1, arg2);
  }
}

struct object *
apply_and_run(struct object *func, struct object *args,
              struct vm_context *ctx)
{
  struct object *result;
  result = apply(func, args, ctx);
  if (result) {
    return result;
  }
  eval_instructions(ctx);
  result = pop_stack(ctx->stk);
  return result;
}
