#ifndef VMSCHEME_COMPILER_H
#define VMSCHEME_COMPILER_H

#include <object.h>
#include <instruction.h>

void init_compiler(void);
struct instruction* compile(struct pair *exprs);

#endif // VMSCHEME_COMPILER_H
