#ifndef VMSCHEME_LOAD_H
#define VMSCHEME_LOAD_H

#include <eval.h>
#include <object.h>

struct object *load(const char *filename, struct vm_context *ctx);
struct object *load_wrap(struct object *filename,
                         struct vm_context *ctx);


#endif // VMSCHEME_LOAD_H
