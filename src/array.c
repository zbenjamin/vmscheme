#include <array.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
array_create(struct array *ar, int elem_size)
{
  ar->elem_size = elem_size;
  ar->size = 0;
  ar->allocated = 0;
  ar->elems = NULL;
}

void
array_dealloc(struct array *ar)
{
  if (ar->elems) {
    free(ar->elems);
  }
}

int
array_size(struct array *ar)
{
  return ar->size;
}

void
array_add(struct array *ar, void *elem)
{
  if (ar->size + 1 > ar->allocated) {
    int new_allocated;
    if (ar->allocated == 0) {
      new_allocated = 16;
    } else {
      new_allocated = ar->allocated * 2;
    }
    uint8_t *new_elems;
    new_elems = (uint8_t*) malloc(new_allocated * ar->elem_size);
    memcpy(new_elems, ar->elems, ar->size * ar->elem_size);
    free(ar->elems);
    ar->allocated = new_allocated;
    ar->elems = new_elems;
  }

  memcpy(ar->elems + ar->size * ar->elem_size, elem, ar->elem_size);
  ++ar->size;
}

void*
array_ref(struct array *ar, int idx)
{
  if (idx >= ar->size) {
    printf("Bad array index: %d (max index is %d)\n", idx, ar->size);
    exit(1);
  }

  return ar->elems + idx * ar->elem_size;
}

void
array_set(struct array *ar, int idx, void *elem)
{
  if (idx >= ar->size) {
    printf("Bad array index: %d (max index is %d)\n", idx, ar->size);
    exit(1);
  }

  memcpy(ar->elems + idx * ar->elem_size, elem, ar->elem_size);
}

void*
array2raw(struct array *ar)
{
  int bytes = ar->size * ar->elem_size;
  void *storage = malloc(bytes);
  memcpy(storage, ar->elems, bytes);
  return storage;
}
