#include <compiler.h>

#include <instruction.h>
#include <object.h>
#include <type.h>
#include <primitive_procedures.h>
#include <utils.h>

#include <stdlib.h>
#include <string.h>

static void compile_to(struct object *exprs,
                       struct instruction **pc);
static void compile_comb_to(struct object *exprs,
                            struct instruction **pc);

struct instruction*
compile(struct object *exprs)
{
  struct instruction *prog;
  prog = malloc(sizeof(struct instruction) * 1024);
  memset(prog, 0, sizeof(struct instruction) * 1024);
  struct instruction *pc = prog;

  compile_to(exprs, &pc);
  pc->op = RET;
  ++pc;
  pc->op = END;
  return prog;
}

void
compile_to(struct object *exprs, struct instruction **pc)
{
  struct object *next = exprs;

  while (next != NIL) {
    struct object *obj = car(next);
    switch (obj->type->code) {
    case NIL_TYPE:
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
      compile_comb_to(obj, pc);
      break;
    }
    next = cdr(next);
  }
}

void
compile_comb_to(struct object *lst, struct instruction **pc)
{
  struct object *first = car(lst);

  if (first->type->code == SYMBOL_TYPE
      && strcmp(first->sval, "define") == 0) {
    // a definition
    compile_to(cdr(cdr(lst)), pc);
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

  // a regular function invocation
  int nargs = list_length_int(lst) - 1;
  compile_to(lst, pc);
  (*pc)->op = PUSH;
  (*pc)->arg = make_integer(nargs);
  ++(*pc);
  (*pc)->op = CALL;
  ++(*pc);
}
