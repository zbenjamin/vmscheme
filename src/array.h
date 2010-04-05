#ifndef VMSCHEME_ARRAY_H
#define VMSCHEME_ARRAY_H

#include <stdint.h>

struct array {
  uint8_t *elems;
  int elem_size;
  int size;
  int allocated;
};

void array_create(struct array *ar, int elem_size);
void array_dealloc(struct array *ar);

int array_size(struct array *ar);
void array_add(struct array *ar, void *elem);
void *array_ref(struct array *ar, int idx);
void array_set(struct array *ar, int idx, void *elem);

void *array2raw(struct array *ar);

#define obj_array_create(ar) \
  array_create((ar), sizeof(struct object*))
#define obj_array_ref(ar, idx) \
  ((struct object **) array_ref((ar), (idx)))

#define str_array_create(ar) \
  array_create((ar), sizeof(char *))
#define str_array_ref(ar, idx) \
  ((char **) array_ref((ar), (idx)))

#endif // VMSCHEME_ARRAY_H
