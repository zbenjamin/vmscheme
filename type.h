#ifndef VMSCHEME_TYPE_H
#define VMSCHEME_TYPE_H

struct type {
  unsigned int code;
  char* name;
};

struct type* get_type(char* name);
struct type* make_type(char* name);

void init_builtin_types();

extern struct type *nil_type;
extern struct type *integer_type;
extern struct type *string_type;
extern struct type *symbol_type;
extern struct type *pair_type;
extern struct type *procedure_type;
extern struct type *primitive_proc_type;

#endif // VMSCHEME_TYPE_H
