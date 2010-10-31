#include <compiler.h>

#include <eval.h>
#include <instruction.h>
#include <load.h>
#include <object.h>
#include <type.h>
#include <primitive_procedures.h>
#include <utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void compile_seq(struct pair *exprs, struct array *prog);
static void compile_list(struct pair *exprs, struct array *prog);
static void compile_elem(struct object *obj, struct array *prog,
                         int tailcall);
static struct object *compile_comb(struct pair *exprs,
                                   struct array *prog,
                                   int tailcall);
static void compile_define(struct pair *lst, struct array *prog);
static void compile_set(struct pair *lst, struct array *prog);
static void compile_lambda(struct pair *lst, struct array *prog);

static void compile_if(struct pair *lst, struct array *prog,
                       int tailcall);

static struct vm_context compiler_ctx;
static int compiler_initialized;
static struct procedure *transform_quasiquote;
static struct procedure *macro_find_and_transform;

void
init_compiler(void)
{
  compiler_initialized = 0;
  compiler_ctx.stk = make_stack(1024);
  compiler_ctx.env = make_environment(global_env);
  compiler_ctx.pc = NULL;
  struct object *value;
  value = load("quasiquote.scm", &compiler_ctx);
  YIELD_OBJ(value);
  value = load("macros.scm", &compiler_ctx);
  YIELD_OBJ(value);

  struct object *res;
  res = env_lookup(compiler_ctx.env, "transform-quasiquote");
  assert(res->type->code == PROCEDURE_TYPE);
  transform_quasiquote = container_of(res, struct procedure, obj);

  res = env_lookup(compiler_ctx.env, "macro:find-and-transform");
  assert(res->type->code == PROCEDURE_TYPE);
  macro_find_and_transform = container_of(res, struct procedure, obj);

  compiler_initialized = 1;
}

void
add_instruction(struct array *prog, enum opcode op,
                struct object *arg)
{
  struct instruction ins;
  ins.op = op;
  ins.arg = arg;
  array_add(prog, &ins);
}

struct instruction*
compile(struct pair *exprs)
{
  struct array prog;
  array_create(&prog, sizeof(struct instruction));

  compile_seq(exprs, &prog);
  add_instruction(&prog, RET, NULL);
  add_instruction(&prog, END, NULL);

  struct instruction *stream;
  stream = (struct instruction*) array2raw(&prog);
  array_dealloc(&prog);
  return stream;
}

// compiles each element, but only keeps the last value
void
compile_seq(struct pair *exprs, struct array *prog)
{
  struct pair *next = exprs;

  while (next != NIL) {
    if (next->cdr == &NIL->obj) {
      compile_elem(next->car, prog, 1);
    } else {
      compile_elem(next->car, prog, 0);
      add_instruction(prog, POP, NULL);
    }
    assert(next->cdr->type->code == PAIR_TYPE
           || next->cdr->type->code == NIL_TYPE);
    next = container_of(next->cdr, struct pair, obj);
  }
}

// just compiles each element
void
compile_list(struct pair *exprs, struct array *prog)
{
  struct pair *next = exprs;

  while (next != NIL) {
    compile_elem(next->car, prog, 0);
    assert(next->cdr->type->code == PAIR_TYPE
           || next->cdr->type->code == NIL_TYPE);
    next = container_of(next->cdr, struct pair, obj);
  }
}

void
compile_elem(struct object *obj, struct array *prog, int tailcall)
{
  struct object *elem = obj;
  while (1) {
    switch (elem->type->code) {
    case NIL_TYPE:
    case BOOLEAN_TYPE:
    case INTEGER_TYPE:
    case STRING_TYPE:
    case PROCEDURE_TYPE:
      add_instruction(prog, PUSH, elem);
      INC_REF(elem);
      return;
    case SYMBOL_TYPE:
      add_instruction(prog, LOOKUP, elem);
      INC_REF(elem);
      return;
    case PAIR_TYPE:
      elem = compile_comb(container_of(elem, struct pair, obj),
                          prog, tailcall);
      if (! elem) {
        return;
      }
      break;
    default:
      printf("Don't know how to compile a '%s'\n", elem->type->name);
      exit(1);
    }
  }
}

// returns a replacement object that should be compiled instead or
// NULL if the compilation of the passed combination is done
struct object *
compile_comb(struct pair *lst, struct array *prog, int tailcall)
{
  struct object *first = lst->car;

  if (first->type->code == SYMBOL_TYPE && compiler_initialized) {
    struct object *result;
    struct pair *args = make_pair(&lst->obj, &NIL->obj);
    if ((result = apply_and_run(macro_find_and_transform, args,
                                &compiler_ctx)) != FALSE) {
      dealloc_obj(&args->obj);
      struct object *new_form = car(result);
      /* useful for debugging macros: */
      /* print_obj(lst); */
      /* printf("\n=>\n"); */
      /* print_obj(new_form); */
      /* printf("\n"); */
      DEC_REF(result);
      return new_form;
    }
    dealloc_obj(&args->obj);
  }

  if (first->type->code == SYMBOL_TYPE) {
    struct symbol *sym = container_of(first, struct symbol, obj);
    if (strcmp(sym->value, "quasiquote") == 0) {
      if (compiler_initialized == 0) {
        printf("Internal error: cannot use advanced compiler "
               "features before compiler is fully initialized\n");
        exit(1);
      }
      lst = make_pair(&lst->obj, &NIL->obj);
      struct object *result;
      result = apply_and_run(transform_quasiquote, lst,
                             &compiler_ctx);
      dealloc_obj(&lst->obj);
      return result;
    }

    if (strcmp(sym->value, "quote") == 0) {
      struct object *datum = car(cdr(&lst->obj));
      add_instruction(prog, PUSH, datum);
      INC_REF(datum);
      return NULL;
    }
    if (strcmp(sym->value, "%define") == 0) {
      compile_define(lst, prog);
      return NULL;
    }
    if (strcmp(sym->value, "set!") == 0) {
      compile_set(lst, prog);
      return NULL;
    }
    if (strcmp(sym->value, "lambda") == 0) {
      compile_lambda(lst, prog);
      return NULL;
    }
    if (strcmp(sym->value, "%if") == 0) {
      compile_if(lst, prog, tailcall);
      return NULL;
    }
  }

  // a regular function invocation
  struct integer *nargs = make_integer(list_length(lst) - 1);
  INC_REF(&nargs->obj);
  compile_list(lst, prog);
  add_instruction(prog, PUSH, &nargs->obj);
  if (tailcall) {
    add_instruction(prog, TAILCALL, NULL);
  } else {
    add_instruction(prog, CALL, NULL);
  }
  return NULL;
}

void
compile_define(struct pair *lst, struct array *prog)
{
  struct pair *rest;
  struct pair *val;
  assert(lst->cdr->type->code == PAIR_TYPE);
  rest = container_of(lst->cdr, struct pair, obj);
  assert(rest->cdr->type->code == PAIR_TYPE);
  val = container_of(rest->cdr, struct pair, obj);
  compile_list(val, prog);
  struct object *name = rest->car;
  add_instruction(prog, DEFINE, name);
  INC_REF(name);
  add_instruction(prog, PUSH, &NIL->obj);
}

void
compile_set(struct pair *lst, struct array *prog)
{
  struct pair *rest;
  struct pair *val;
  assert(lst->cdr->type->code == PAIR_TYPE);
  rest = container_of(lst->cdr, struct pair, obj);
  assert(rest->cdr->type->code == PAIR_TYPE);
  val = container_of(rest->cdr, struct pair, obj);
  compile_list(val, prog);
  struct object *name = rest->car;
  add_instruction(prog, SET, name);
  INC_REF(name);
  add_instruction(prog, PUSH, &NIL->obj);
}

void
compile_lambda(struct pair *lst, struct array *prog)
{
  struct pair *rest;
  struct object *args;
  struct pair *body;
  assert(lst->cdr->type->code == PAIR_TYPE);
  rest = container_of(lst->cdr, struct pair, obj);
  assert(rest->cdr->type->code == PAIR_TYPE);
  args = rest->car;
  body = container_of(rest->cdr, struct pair, obj);
  struct compound_proc *template;
  template = make_compound_procedure(args,
                                     make_code(compile(body)),
                                     NULL);
  add_instruction(prog, LAMBDA, &template->proc.obj);
  INC_REF(&template->proc.obj);
}

void
compile_if(struct pair *lst, struct array *prog, int tailcall)
{
  // turn the two clauses into lambdas

  // (if test conseq alt)
  struct pair *p;
  assert(lst->cdr->type->code == PAIR_TYPE);
  p = container_of(lst->cdr, struct pair, obj);
  struct pair *test = make_pair(p->car, &NIL->obj);

  assert(p->cdr->type->code == PAIR_TYPE);
  p = container_of(p->cdr, struct pair, obj);
  struct compound_proc *conseq;
  struct pair *conseq_code = make_pair(p->car, &NIL->obj);
  conseq = make_compound_procedure(&NIL->obj,
                                   make_code(compile(conseq_code)),
                                   NULL);
  INC_REF(&conseq->proc.obj);
  dealloc_obj(&conseq_code->obj);

  assert(p->cdr->type->code == PAIR_TYPE);
  p = container_of(p->cdr, struct pair, obj);
  struct compound_proc *alt;
  struct pair *alt_code = make_pair(p->car, &NIL->obj);
  alt = make_compound_procedure(&NIL->obj,
                                make_code(compile(alt_code)),
                                NULL);
  INC_REF(&alt->proc.obj);
  dealloc_obj(&alt_code->obj);

  add_instruction(prog, LAMBDA, &conseq->proc.obj);
  add_instruction(prog, LAMBDA, &alt->proc.obj);
  compile_list(test, prog);
  dealloc_obj(&test->obj);
  if (tailcall) {
    add_instruction(prog, TAILIF, NULL);
  } else {
    add_instruction(prog, IF, NULL);
  }
}
