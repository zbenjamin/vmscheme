#ifndef VMSCHEME_COMPILER_H
#define VMSCHEME_COMPILER_H

#include <object.h>
#include <instruction.h>

void init_compiler();
struct instruction* compile(struct object *exprs);

#endif // VMSCHEME_COMPILER_H
