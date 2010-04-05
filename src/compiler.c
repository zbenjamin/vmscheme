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

static void compile_seq(struct object *exprs, struct array *prog);
static void compile_list(struct object *exprs, struct array *prog);
static void compile_elem(struct object *exprs, struct array *prog,
                         int tailcall);
static struct object *compile_comb(struct object *exprs,
                                   struct array *prog,
                                   int tailcall);

static struct vm_context compiler_ctx;
static int compiler_initialized;
static struct object *transform_quasiquote;
static struct object *macro_find_and_transform;

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

  transform_quasiquote = env_lookup(compiler_ctx.env,
                                    "transform-quasiquote");
  macro_find_and_transform = env_lookup(compiler_ctx.env,
                                        "macro:find-and-transform");
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
compile(struct object *exprs)
{
  struct array *prog = malloc(sizeof(struct array));
  array_create(prog, sizeof(struct instruction));

  compile_seq(exprs, prog);
  add_instruction(prog, RET, NULL);
  add_instruction(prog, END, NULL);

  struct instruction *stream = array2raw(prog);
  array_dealloc(prog);
  free(prog);
  return stream;
}

// compiles each element, but only keeps the last value
void
compile_seq(struct object *exprs, struct array *prog)
{
  struct object *next = exprs;

  while (next != NIL) {
    if (cdr(next) == NIL) {
      compile_elem(car(next), prog, 1);
    } else {
      compile_elem(car(next), prog, 0);
      add_instruction(prog, POP, NULL);
    }
    next = cdr(next);
  }
}

// just compiles each element
void
compile_list(struct object *exprs, struct array *prog)
{
  struct object *next = exprs;

  while (next != NIL) {
    compile_elem(car(next), prog, 0);
    next = cdr(next);
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
    case PRIMITIVE_PROC_TYPE:
      add_instruction(prog, PUSH, elem);
      INC_REF(elem);
      return;
    case SYMBOL_TYPE:
      add_instruction(prog, LOOKUP, elem);
      INC_REF(elem);
      return;
    case PAIR_TYPE:
      elem = compile_comb(elem, prog, tailcall);
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
compile_comb(struct object *lst, struct array *prog, int tailcall)
{
  struct object *first = car(lst);

  if (first->type->code == SYMBOL_TYPE && compiler_initialized) {
    struct object *result;
    struct object *args = make_pair(lst, NIL);
    if ((result = apply_and_run(macro_find_and_transform, args,
                                &compiler_ctx)) != FALSE) {
      dealloc_obj(args);
      struct object *new_form = car(result);
      DEC_REF(result);
      return new_form;
    }
    dealloc_obj(args);
  }

  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "quasiquote") == 0) {
    if (compiler_initialized == 0) {
      printf("Internal error: cannot use advanced compiler "
             "features before compiler is fully initialized\n");
      exit(1);
    }
    lst = make_pair(lst, NIL);
    struct object *result;
    result = apply_and_run(transform_quasiquote, lst, &compiler_ctx);
    dealloc_obj(lst);
    return result;
  }

  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "quote") == 0) {
    struct object *datum = car(cdr(lst));
    add_instruction(prog, PUSH, datum);
    INC_REF(datum);
    return NULL;
  }
  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "define") == 0) {
    // a definition
    compile_list(cdr(cdr(lst)), prog);
    struct object *name = car(cdr(lst));
    add_instruction(prog, DEFINE, name);
    INC_REF(name);
    add_instruction(prog, PUSH, NIL);
    return NULL;
  }
  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "set!") == 0) {
    // a definition
    compile_list(cdr(cdr(lst)), prog);
    struct object *name = car(cdr(lst));
    add_instruction(prog, SET, name);
    INC_REF(name);
    add_instruction(prog, PUSH, NIL);
    return NULL;
  }
  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "lambda") == 0) {
    // a lambda expression
    struct object *template;
    template = make_procedure(car(cdr(lst)),
                              make_code(compile(cdr(cdr(lst)))),
                              NULL);
    add_instruction(prog, LAMBDA, template);
    INC_REF(template);
    return NULL;
  }
  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "if") == 0) {
    // an if statement.  turn the two clauses into lambdas
    struct object *conseq;
    struct object *conseq_code = make_pair(car(cdr(cdr(lst))), NIL);
    conseq = make_procedure(NIL,
                            make_code(compile(conseq_code)),
                            NULL);
    INC_REF(conseq);
    dealloc_obj(conseq_code);
    struct object *alt;
    struct object *alt_code = make_pair(car(cdr(cdr(cdr(lst)))), NIL);
    alt = make_procedure(NIL,
                         make_code(compile(alt_code)),
                         NULL);
    INC_REF(alt);
    dealloc_obj(alt_code);
    add_instruction(prog, LAMBDA, conseq);
    add_instruction(prog, LAMBDA, alt);
    struct object *test = make_pair(car(cdr(lst)), NIL);
    compile_list(test, prog);
    dealloc_obj(test);
    if (tailcall) {
      add_instruction(prog, TAILIF, NULL);
    } else {
      add_instruction(prog, IF, NULL);
    }
    return NULL;
  }

  // a regular function invocation
  int nargs = list_length_int(lst) - 1;
  struct object *obj_nargs = make_integer(nargs);
  INC_REF(obj_nargs);
  compile_list(lst, prog);
  add_instruction(prog, PUSH, obj_nargs);
  if (tailcall) {
    add_instruction(prog, TAILCALL, NULL);
  } else {
    add_instruction(prog, CALL, NULL);
  }
  return NULL;
}
