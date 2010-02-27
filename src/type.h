#ifndef VMSCHEME_TYPE_H
#define VMSCHEME_TYPE_H

struct type {
  unsigned int code;
  char* name;
};

struct type* get_type(int typeid);
struct type* get_type_named(char* name);
struct type* make_type(char* name);

void init_builtin_types();

enum builtin_types {
  NIL_TYPE,
  INTEGER_TYPE,
  STRING_TYPE,
  SYMBOL_TYPE,
  PAIR_TYPE,
  PROCEDURE_TYPE,
  PRIMITIVE_PROC_TYPE,
  CODE_TYPE,
  ENVIRONMENT_TYPE
};

#endif // VMSCHEME_TYPE_H
