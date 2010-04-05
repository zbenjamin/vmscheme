#ifndef VMSCHEME_SYMBOL_H
#define VMSCHEME_SYMBOL_H

#include <array.h>
#include <object.h>

struct array symbol_table;

void init_symbol_table(void);
struct object *get_symbol(char *name);

#endif // VMSCHEME_SYMBOL_H
