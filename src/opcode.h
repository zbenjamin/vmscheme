#ifndef VMSCHEME_OPCODE_H
#define VMSCHEME_OPCODE_H

enum opcode {
  NONE,
  END,
  PUSH,
  LOOKUP,
  CALL,
  RET,
  DEFINE,
  LAMBDA
};

#endif // VMSCHEME_OPCODE_H
