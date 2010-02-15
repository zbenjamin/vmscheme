#ifndef VMSCHEME_INSTRUCTION_H
#define VMSCHEME_INSTRUCTION_H

#include <opcode.h>
#include <object.h>

struct instruction {
  enum opcode op;
  struct object *arg;
};

#endif // VMSCHEME_INSTRUCTION_H
