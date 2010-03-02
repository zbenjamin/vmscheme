#ifndef VMSCHEME_OPCODE_H
#define VMSCHEME_OPCODE_H

enum opcode {
  NONE,
  END,
  PUSH,
  POP,
  LOOKUP,
  CALL,
  RET,
  DEFINE,
  LAMBDA,
  IF
};

#endif // VMSCHEME_OPCODE_H
