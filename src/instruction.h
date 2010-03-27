#ifndef VMSCHEME_INSTRUCTION_H
#define VMSCHEME_INSTRUCTION_H

#include <opcode.h>
#include <object.h>

struct instruction {
  enum opcode op;
  struct object *arg;
};

void dealloc_bytecode(struct instruction *stream);
struct object *disassemble(struct object *proc);

#endif // VMSCHEME_INSTRUCTION_H
