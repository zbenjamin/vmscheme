#ifndef VMSCHEME_TYPE_H
#define VMSCHEME_TYPE_H

struct type {
  unsigned int code;
  char* name;
};

struct type* get_type(int typeid);
struct type* get_type_named(char* name);
struct type* make_type(char* name);

void init_builtin_types(void);

enum builtin_types {
  NIL_TYPE,
  UNSPECIFIC_TYPE,
  BOOLEAN_TYPE,
  INTEGER_TYPE,
  STRING_TYPE,
  SYMBOL_TYPE,
  PAIR_TYPE,
  PROCEDURE_TYPE,
  CODE_TYPE,
  CODEPTR_TYPE,
  ENVIRONMENT_TYPE,
  CONTINUATION_TYPE
};

#endif // VMSCHEME_TYPE_H
