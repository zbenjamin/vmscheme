#include <compiler.h>

#include <instruction.h>
#include <object.h>
#include <type.h>
#include <primitive_procedures.h>
#include <utils.h>

#include <stdlib.h>
#include <string.h>

static void compile_seq(struct object *exprs,
                        struct instruction **pc);
static void compile_list(struct object *exprs,
                         struct instruction **pc);
static void compile_elem(struct object *exprs,
                         struct instruction **pc);
static void compile_comb(struct object *exprs,
                         struct instruction **pc);

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
  switch (obj->type->code) {
  case NIL_TYPE:
  case BOOLEAN_TYPE:
  case INTEGER_TYPE:
  case STRING_TYPE:
    (*pc)->op = PUSH;
    (*pc)->arg = obj;
    ++(*pc);
    break;
  case SYMBOL_TYPE:
    (*pc)->op = LOOKUP;
    (*pc)->arg = obj;
    ++(*pc);
    break;
  case PAIR_TYPE:
    compile_comb(obj, pc);
    break;
  }
}

void
compile_comb(struct object *lst, struct instruction **pc)
{
  struct object *first = car(lst);

  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "define") == 0) {
    // a definition
    compile_list(cdr(cdr(lst)), pc);
    (*pc)->op = DEFINE;
    (*pc)->arg = car(cdr(lst));
    ++(*pc);
    (*pc)->op = PUSH;
    (*pc)->arg = NIL;
    ++(*pc);
    return;
  }
  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "lambda") == 0) {
    // a lambda expression
    struct object *template;
    template = make_procedure(car(cdr(lst)),
                              make_code(compile(cdr(cdr(lst)))),
                              NULL);
    (*pc)->op = LAMBDA;
    (*pc)->arg = template;
    ++(*pc);
    return;
  }
  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "if") == 0) {
    // an if statement.  turn the two clauses into lambdas
    struct object *conseq;
    struct object *conseq_code = make_pair(car(cdr(cdr(lst))), NIL);
    conseq = make_procedure(NIL,
                            make_code(compile(conseq_code)),
                            NULL);
    struct object *alt;
    struct object *alt_code = make_pair(car(cdr(cdr(cdr(lst)))), NIL);
    alt = make_procedure(NIL,
                         make_code(compile(alt_code)),
                         NULL);
    (*pc)->op = LAMBDA;
    (*pc)->arg = conseq;
    ++(*pc);
    (*pc)->op = LAMBDA;
    (*pc)->arg = alt;
    ++(*pc);
    compile_list(make_pair(car(cdr(lst)), NIL), pc);
    (*pc)->op = IF;
    ++(*pc);
    return;
  }

  // a regular function invocation
  int nargs = list_length_int(lst) - 1;
  compile_list(lst, pc);
  (*pc)->op = PUSH;
  (*pc)->arg = make_integer(nargs);
  ++(*pc);
  (*pc)->op = CALL;
  ++(*pc);
}
