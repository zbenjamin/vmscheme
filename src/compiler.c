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

struct instruction*
compile(struct object *exprs)
{
  struct instruction *prog;
  prog = malloc(sizeof(struct instruction) * 1024);
  memset(prog, 0, sizeof(struct instruction) * 1024);
  struct instruction *pc = prog;

  compile_to(exprs, &pc);
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
      compile_to(obj, pc);
      (*pc)->op = PUSH;
      (*pc)->arg = make_integer(list_length_int(obj) - 1);
      ++(*pc);
      (*pc)->op = CALL;
      ++(*pc);
      break;
    }
    next = cdr(next);
  }
}
