#include <instruction.h>
#include <object.h>

#include <stdlib.h>

void
dealloc_bytecode(struct instruction *stream)
{
  struct instruction *pc = stream;
  while (pc->op != END) {
    if (pc->arg) {
      DEC_REF(pc->arg);
    }
    ++pc;
  };

  free(stream);
}
