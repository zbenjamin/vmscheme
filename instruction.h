#ifndef VMSCHEME_INSTRUCTION_H
#define VMSCHEME_INSTRUCTION_H

#include <opcode.h>

struct instruction {
  enum opcode op;
  void *arg;
};

#endif // VMSCHEME_INSTRUCTION_H
