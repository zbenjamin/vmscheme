#ifndef VMSCHEME_INSTRUCTION_H
#define VMSCHEME_INSTRUCTION_H

#include <opcode.h>
#include <object.h>

struct instruction {
  struct object obj;
  enum opcode op;
  struct object *arg;
};

void dealloc_bytecode(struct instruction *stream);
struct object *disassemble_wrap(struct object *proc);
void disassemble(struct instruction *stream);

#endif // VMSCHEME_INSTRUCTION_H
