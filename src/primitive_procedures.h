#ifndef VMSCHEME_PRIMITIVE_PROCEDURES_H
#define VMSCHEME_PRIMITIVE_PROCEDURES_H

#include <object.h>

void init_primitive_procs();

struct object *car(struct object *pair);
struct object *cdr(struct object *pair);
struct object *reverse_list(struct object *lst);
struct object *list_length(struct object *lst);
struct object *plus(struct object *n1, struct object *n2);
struct object *minus(struct object *n1, struct object *n2);

#endif // VMSCHEME_PRIMITIVE_PROCEDURES_H
