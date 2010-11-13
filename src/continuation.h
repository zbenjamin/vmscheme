#ifndef VMSCHEME_CONTINUATION_H
#define VMSCHEME_CONTINUATION_H

#include <object.h>
#include <eval.h>

void dealloc_continuation(struct vm_context *cont);
struct object *call_cc(struct object *proc, struct vm_context **ctx);
struct object *invoke_continuation(struct object *cont,
                                   struct object *value,
                                   struct vm_context **ctx);

#endif // VMSCHEME_CONTINUATION_H
