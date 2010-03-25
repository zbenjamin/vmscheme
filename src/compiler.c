#include <compiler.h>

#include <eval.h>
#include <instruction.h>
#include <object.h>
#include <type.h>
#include <parser_aux.h>
#include <primitive_procedures.h>
#include <utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void compile_seq(struct object *exprs,
                        struct instruction **pc);
static void compile_list(struct object *exprs,
                         struct instruction **pc);
static void compile_elem(struct object *exprs,
                         struct instruction **pc);
static struct object *compile_comb(struct object *exprs,
                                   struct instruction **pc);

static struct vm_context compiler_ctx;
static int compiler_initialized;
static struct object *transform_quasiquote;

void
init_compiler()
{
  compiler_initialized = 0;
  compiler_ctx.stk = make_stack(1024);
  compiler_ctx.env = make_environment(global_env);
  compiler_ctx.pc = NULL;
  struct object *forms = parse_file("quasiquote.scm");
  // shouldn't need to deallocate the return value of eval_sequence
  // because it should be '()
  eval_sequence(forms,
                compiler_ctx.env);
  dealloc_obj(forms);
  transform_quasiquote = env_lookup(compiler_ctx.env,
                                    "transform-quasiquote");
  compiler_initialized = 1;
}

struct instruction*
compile(struct object *exprs)
{
  struct instruction *prog;
  prog = malloc(sizeof(struct instruction) * 1024);
  memset(prog, 0, sizeof(struct instruction) * 1024);
  struct instruction *pc = prog;

  compile_seq(exprs, &pc);
  pc->op = RET;
  ++pc;
  pc->op = END;
  return prog;
}

// compiles each element, but only keeps the last value
void
compile_seq(struct object *exprs, struct instruction **pc)
{
  struct object *next = exprs;

  while (next != NIL) {
    compile_elem(car(next), pc);
    if (cdr(next) != NIL) {
      (*pc)->op = POP;
      ++(*pc);
    }
    next = cdr(next);
  }
}

// just compiles each element
void
compile_list(struct object *exprs, struct instruction **pc)
{
  struct object *next = exprs;

  while (next != NIL) {
    compile_elem(car(next), pc);
    next = cdr(next);
  }
}

void
compile_elem(struct object *obj, struct instruction **pc)
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
      (*pc)->op = PUSH;
      INC_REF(elem);
      (*pc)->arg = elem;
      ++(*pc);
      return;
    case SYMBOL_TYPE:
      (*pc)->op = LOOKUP;
      INC_REF(elem);
      (*pc)->arg = elem;
      ++(*pc);
      return;
    case PAIR_TYPE:
      elem = compile_comb(elem, pc);
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
compile_comb(struct object *lst, struct instruction **pc)
{
  struct object *first = car(lst);

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
    (*pc)->op = PUSH;
    struct object *datum = car(cdr(lst));
    INC_REF(datum);
    (*pc)->arg = datum;
    ++(*pc);
    return NULL;
  }
  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "define") == 0) {
    // a definition
    compile_list(cdr(cdr(lst)), pc);
    (*pc)->op = DEFINE;
    struct object *name = car(cdr(lst));
    INC_REF(name);
    (*pc)->arg = name;
    ++(*pc);
    (*pc)->op = PUSH;
    (*pc)->arg = NIL;
    ++(*pc);
    return NULL;
  }
  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "lambda") == 0) {
    // a lambda expression
    struct object *template;
    template = make_procedure(car(cdr(lst)),
                              make_code(compile(cdr(cdr(lst)))),
                              NULL);
    INC_REF(template);
    (*pc)->op = LAMBDA;
    (*pc)->arg = template;
    ++(*pc);
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
    (*pc)->op = LAMBDA;
    (*pc)->arg = conseq;
    ++(*pc);
    (*pc)->op = LAMBDA;
    (*pc)->arg = alt;
    ++(*pc);
    struct object *test = make_pair(car(cdr(lst)), NIL);
    compile_list(test, pc);
    dealloc_obj(test);
    (*pc)->op = IF;
    ++(*pc);
    return NULL;
  }

  // a regular function invocation
  int nargs = list_length_int(lst) - 1;
  struct object *obj_nargs = make_integer(nargs);
  INC_REF(obj_nargs);
  compile_list(lst, pc);
  (*pc)->op = PUSH;
  (*pc)->arg = obj_nargs;
  ++(*pc);
  (*pc)->op = CALL;
  ++(*pc);
  return NULL;
}
