#ifndef VMSCHEME_INSTRUCTION_H
#define VMSCHEME_INSTRUCTION_H

#include <opcode.h>
#include <object.h>

struct instruction {
  enum opcode op;
  struct object *arg;
};

struct code {
  struct object obj;
  struct instruction *stream;
};

struct codeptr {
  struct object obj;
  struct code *base;
  size_t offset;
};

void dealloc_bytecode(struct instruction *stream);
struct object *disassemble_wrap(struct object *proc);
void disassemble(struct code *stream);

#endif // VMSCHEME_INSTRUCTION_H
