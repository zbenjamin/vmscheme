#include <eval.h>

#include <compiler.h>
#include <debug.h>
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
static struct procedure* check_proc(struct object *func);
static struct object *primitive_apply0(struct prim_proc *func,
                                       struct pair *args,
                                       struct vm_context *ctx);
static struct object *primitive_apply1(struct prim_proc *func,
                                       struct pair *args,
                                       struct vm_context *ctx);
static struct object *primitive_apply2(struct prim_proc *func,
                                       struct pair *args,
                                       struct vm_context *ctx);


// see note for eval_sequence, below
struct object *
eval(struct pair *form, struct environment *env)
{
  struct pair *seq = make_pair(&form->obj, &NIL->obj);
  struct object *ret = eval_sequence(seq, env);
  dealloc_obj(&seq->obj);
  return ret;
}

// eval_sequence may return an object with a refcount of zero.
// This is because it's the interface to the interpreter and so the
// return value may be the result of a computation that only has
// one reference inside the created vm context.  On the other hand
// it may return an object with multiple references (if the object
// returned is referenced in the passed environment, for example).
struct object *
eval_sequence(struct pair *forms, struct environment *env)
{
  struct instruction *prog = compile(forms);
  struct stack *stk = make_stack(1024);
  // push magic "end of instructions" return address
  stack_push(stk, NULL);
  stack_push(stk, &env->obj);
  INC_REF(&env->obj);
  struct vm_context ctx = { prog, stk, env };

  eval_instructions(&ctx);
  struct object *value = stack_pop(stk);

  // decrement the refcount if it's positive, but don't deallocate
  // the object
  if (value->refcount > 0) {
    --(value->refcount);
  }

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
  struct symbol *sym;
  struct object *value;
  struct compound_proc *template;

  switch (ctx->pc->op) {
  case NONE:
    printf("Error: tried to execute a NONE op\n");
    exit(1);
    break;
  case PUSH:
    /* printf("PUSH instruction\n"); */
    stack_push(ctx->stk, ctx->pc->arg);
    INC_REF(ctx->pc->arg);
    ++ctx->pc;
    break;
  case POP:
    /* printf("POP instruction\n"); */
    value = stack_pop(ctx->stk);
    DEC_REF(value);
    ++ctx->pc;
    break;
  case LOOKUP:
    /* printf("LOOKUP instruction\n"); */
    assert(ctx->pc->arg->type->code == SYMBOL_TYPE);
    sym = container_of(ctx->pc->arg, struct symbol, obj);
    value = env_lookup(ctx->env, sym->value);
    if (! value) {
      char buf[1024];
      debug_loc_str(ctx->pc->arg, buf, 1024);
      printf("%s: unbound name: %s\n", buf, sym->value);
      exit(1);
    }
    stack_push(ctx->stk, value);
    INC_REF(value);
    ++ctx->pc;
    break;
  case CALL:
  case TAILCALL:
    /* printf("CALL instruction @ %p\n", *pc); */
    eval_call(ctx);
    break;
  case RET:
    value = stack_pop(ctx->stk);
    struct object *orig_env = stack_pop(ctx->stk);
    assert(orig_env->type->code == ENVIRONMENT_TYPE);
    DEC_REF(orig_env);
    struct object *retaddr = stack_pop(ctx->stk);
    /* printf("RET instruction @ %p to %p\n", *pc, retaddr->cval); */
    stack_push(ctx->stk, value);
    DEC_REF(&ctx->env->obj);
    ctx->env = container_of(orig_env, struct environment, obj);
    if (retaddr == NULL) {
      ctx->pc = 0;
      return 1;
    }
    assert(retaddr->type->code == CODE_TYPE);
    ctx->pc = container_of(retaddr, struct code, obj)->ins;
    free(retaddr);
    break;
  case DEFINE:
    /* printf("DEFINE instruction\n"); */
    value = stack_pop(ctx->stk);
    assert(ctx->pc->arg->type->code == SYMBOL_TYPE);
    sym = container_of(ctx->pc->arg, struct symbol, obj);
    env_define(ctx->env, sym->value, value);
    DEC_REF(value);
    ++ctx->pc;
    break;
  case SET:
    value = stack_pop(ctx->stk);
    assert(ctx->pc->arg->type->code == SYMBOL_TYPE);
    sym = container_of(ctx->pc->arg, struct symbol, obj);
    env_set(ctx->env, sym->value, value);
    DEC_REF(value);
    ++ctx->pc;
    break;
  case LAMBDA:
    /* printf("LAMBDA instruction\n"); */
    value = ctx->pc->arg;
    assert(ctx->pc->arg->type->code == PROCEDURE_TYPE);
    template = container_of(container_of(ctx->pc->arg,
                                         struct procedure, obj),
                            struct compound_proc, proc);
    struct compound_proc *proc;
    proc = make_compound_procedure(template->params,
                                   template->code,
                                   ctx->env);
    stack_push(ctx->stk, &proc->proc.obj);
    INC_REF(&proc->proc.obj);
    ++ctx->pc;
    break;
  case IF:
  case TAILIF:
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
  struct object *num_args = stack_pop(ctx->stk);
  struct pair *args = NIL;
  if (num_args->type->code != INTEGER_TYPE) {
    printf("Internal error: number of arguments to call "
           "is not an integer\n");
    exit(1);
  }

  int num = container_of(num_args, struct integer, obj)->value;
  DEC_REF(num_args);
  while (num) {
    struct object *arg = stack_pop(ctx->stk);
    args = make_pair(arg, &args->obj);
    DEC_REF(arg);
    --num;
  }

  struct procedure *func = check_proc(stack_pop(ctx->stk));
  struct object *result;
  result = apply(func, args, ctx);

  // we get a result back for primitive functions, but compound
  // functions muck with the vm context instead
  if (result != NULL) {
    ++ctx->pc;
    stack_push(ctx->stk, result);
    // need to increment the refcount of the result before
    // deallocating the arguments in case the function returns some
    // bit of the arguments (like car or cdr)
    INC_REF(result);
  }

  YIELD_OBJ(&args->obj);
  DEC_REF(&func->obj);
}

void
eval_if(struct vm_context *ctx)
{
  struct object *testval = stack_pop(ctx->stk);
  struct object *alt = stack_pop(ctx->stk);
  struct object *conseq = stack_pop(ctx->stk);

  struct object *action;
  if (testval == FALSE) {
    action = alt;
    DEC_REF(conseq);
  } else {
    action = conseq;
    DEC_REF(alt);
  }

  struct integer *zero = make_integer(0);
  stack_push(ctx->stk, action);
  stack_push(ctx->stk, &zero->obj);
  INC_REF(&zero->obj);
  eval_call(ctx);
}

struct procedure*
check_proc(struct object *func)
{
  if (func->type->code != PROCEDURE_TYPE) {
    char buf[1024];
    debug_loc_str(func, buf, 1024);
    printf("Cannot apply object of type %s, created at %s\n",
           func->type->name, buf);
    exit(1);
  }

  return container_of(func, struct procedure, obj);
}

struct pair*
check_args(struct object *args)
{
  if (args->type->code != PAIR_TYPE && args->type->code != NIL_TYPE) {
    char buf[1024];
    debug_loc_str(args, buf, 1024);
    printf("Cannot use object of type %s, created at %s, "
           "as an argument list\n",
           args->type->name, buf);
    exit(1);
  }

  return container_of(args, struct pair, obj);
}

struct object *
apply(struct procedure *func, struct pair *args,
      struct vm_context *ctx)
{
  unsigned int num_args = list_length(args);

  struct compound_proc *cp;
  struct prim_proc *pp;
  int wrong_args;
  unsigned int num_params = 0;
  if (func->type == COMPOUND) {
    cp = container_of(func, struct compound_proc, proc);
    struct object *rest_params = cp->params;
    int var_params = 0;
    while (rest_params != &NIL->obj) {
      if (rest_params->type->code == SYMBOL_TYPE) {
        var_params = 1;
        break;
      }
      ++num_params;
      assert(rest_params->type->code == PAIR_TYPE);
      rest_params = container_of(rest_params, struct pair, obj)->cdr;
    }

    if (var_params) {
      wrong_args = (num_args < num_params);
    } else {
      wrong_args = (num_args != num_params);
    }
  } else {
    // primitive procedure
    pp = container_of(func, struct prim_proc, proc);
    num_params = pp->arity;
    wrong_args = (num_args != num_params);
  }

  if (wrong_args) {
      printf("Incorrect number of arguments to ");
      print_obj(&func->obj);
      printf("\n");
      exit(1);
  }

  if (func->type == COMPOUND) {
    // Push the return value onto the stack.  We don't want the
    // return addr garbage collected.  Also account for the case that
    // we were called from apply_and_run (and thus the pc might be
    // zero).
    struct code *retaddr;
    if (! ctx->pc) {
      retaddr = NULL;
      stack_push(ctx->stk, &retaddr->obj);
      stack_push(ctx->stk, &ctx->env->obj);
      INC_REF(&ctx->env->obj);
    } else if (ctx->pc->op == CALL || ctx->pc->op == IF) {
      retaddr = make_code(ctx->pc + 1);
      retaddr->obj.refcount = -1;
      stack_push(ctx->stk, &retaddr->obj);
      stack_push(ctx->stk, &ctx->env->obj);
      INC_REF(&ctx->env->obj);
    } else {
      assert(ctx->pc->op == TAILCALL || ctx->pc->op == TAILIF);
      // a tail call of some kind
      DEC_REF(&ctx->env->obj);
    }

    struct environment *new_env;
    new_env = make_environment(cp->env);
    env_bind_names(new_env, cp->params, args);
    ctx->pc = cp->code->ins;
    ctx->env = new_env;
    INC_REF(&new_env->obj);
    return NULL;
  }

  struct object *result;
  // primitive function dispatch
  switch (num_args) {
  case 0:
    result = primitive_apply0(pp, args, ctx);
    break;
  case 1:
    result = primitive_apply1(pp, args, ctx);
    break;
  case 2:
    result = primitive_apply2(pp, args, ctx);
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
primitive_apply0(struct prim_proc *func, struct pair *args,
                 struct vm_context *ctx)
{
  if (func->takes_ctx) {
    struct object *(*function)(struct vm_context*);
    function = (struct object *(*)(struct vm_context*)) func->func;
    return function(ctx);
  } else {
    struct object *(*function)();
    function = (struct object *(*)()) func->func;
    return function();
  }
}

struct object *
primitive_apply1(struct prim_proc *func, struct pair *args,
                 struct vm_context *ctx)
{
  struct object *arg1 = args->car;

  if (func->takes_ctx) {
    struct object *(*function)(struct object*, struct vm_context*);
    function = (struct object *(*)(struct object*,
                                   struct vm_context*)) func->func;
    return function(arg1, ctx);
  } else {
    struct object *(*function)(struct object*);
    function = (struct object *(*)(struct object*)) func->func;
    return function(arg1);
  }
}

struct object *
primitive_apply2(struct prim_proc *func, struct pair *args,
                 struct vm_context *ctx)
{
  struct object *arg1 = args->car;
  assert(args->cdr->type->code == PAIR_TYPE);
  args = container_of(args->cdr, struct pair, obj);
  struct object *arg2 = args->car;

  if (func->takes_ctx) {
    struct object *(*function)(struct object*,
                               struct object*,
                               struct vm_context*);
    function = (struct object *(*)(struct object*,
                                   struct object*,
                                   struct vm_context*)) func->func;
    return function(arg1, arg2, ctx);
  } else {
    struct object *(*function)(struct object*,
                               struct object*);
    function = (struct object *(*)(struct object*,
                                   struct object*)) func->func;
    return function(arg1, arg2);
  }
}

struct object *
apply_wrap(struct object *func, struct object *args,
           struct vm_context *ctx)
{
  return apply(check_proc(func), check_args(args), ctx);
}

struct object *
apply_and_run(struct procedure *func, struct pair *args,
              struct vm_context *ctx)
{
  struct object *result;
  result = apply(func, args, ctx);
  if (result) {
    return result;
  }
  eval_instructions(ctx);
  result = stack_pop(ctx->stk);
  return result;
}
