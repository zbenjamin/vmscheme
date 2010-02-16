#ifndef VMSCHEME_INSTRUCTION_H
#define VMSCHEME_INSTRUCTION_H

#include <opcode.h>
#include <object.h>

struct instruction {
  enum opcode op;
  struct object *arg;
};

void dealloc_bytecode(struct instruction *stream);

#endif // VMSCHEME_INSTRUCTION_H
