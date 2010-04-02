#ifndef VMSCHEME_OPCODE_H
#define VMSCHEME_OPCODE_H

enum opcode {
  NONE,
  END,
  PUSH,
  POP,
  LOOKUP,
  CALL,
  TAILCALL,
  RET,
  DEFINE,
  SET,
  LAMBDA,
  IF,
  TAILIF
};

#endif // VMSCHEME_OPCODE_H
