#ifndef VMSCHEME_PRIMITIVE_PROCEDURES_H
#define VMSCHEME_PRIMITIVE_PROCEDURES_H

#include <object.h>

void init_primitive_procs(void);

struct vm_context;

struct object *car(const struct object *pair);
struct object *cdr(const struct object *pair);
struct object *set_car(struct object *pair, struct object *val);
struct object *set_cdr(struct object *pair, struct object *val);
struct object *reverse_list(struct object *lst);
struct object *last_pair(struct object *pair);
struct object *list_length(struct object *lst);
struct object *plus(struct object *n1, struct object *n2);
struct object *minus(struct object *n1, struct object *n2);
struct object *mult(struct object *n1, struct object *n2);
struct object *idiv(struct object *n1, struct object *n2);
struct object *iequal(struct object *n1, struct object *n2);
struct object *eq_p(struct object *o1, struct object *o2);
struct object *eqv_p(struct object *o1, struct object *o2);
struct object *object_type(struct object *obj);
struct object *display(struct object *obj);

#endif // VMSCHEME_PRIMITIVE_PROCEDURES_H
