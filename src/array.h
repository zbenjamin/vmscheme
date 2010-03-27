#ifndef VMSCHEME_ARRAY_H
#define VMSCHEME_ARRAY_H

struct array {
  void **elems;
  int size;
  int allocated;
};

void array_create(struct array *ar);
void array_dealloc(struct array *ar, int do_free);

int array_size(struct array *ar);
void array_add(struct array *ar, void *elem);
void *array_ref(struct array *ar, int idx);
void array_set(struct array *ar, int idx, void *elem);

#endif // VMSCHEME_ARRAY_H
