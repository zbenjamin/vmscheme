#include <array.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
array_create(struct array *ar)
{
  ar->size = 0;
  ar->allocated = 0;
  ar->elems = NULL;
}

void
array_dealloc(struct array *ar, int do_free)
{
  if (do_free) {
    int i;
    for (i = 0; i < ar->size; ++i) {
      free(ar->elems[i]);
    }
  }
  free(ar->elems);
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
      new_allocated = 10;
    } else {
      new_allocated = ar->allocated * 2;
    }
    void **new_elems = malloc(sizeof(void*) * new_allocated);
    memcpy(new_elems, ar->elems, sizeof(void*) * ar->size);
    free(ar->elems);
    ar->allocated = new_allocated;
    ar->elems = new_elems;
  }

  ar->elems[ar->size++] = elem;
}

void*
array_ref(struct array *ar, int idx)
{
  if (idx >= ar->size) {
    printf("Bad array index: %d (max index is %d)\n", idx, ar->size);
    exit(1);
  }

  return ar->elems[idx];
}

void
array_set(struct array *ar, int idx, void *elem)
{
  if (idx >= ar->size) {
    printf("Bad array index: %d (max index is %d)\n", idx, ar->size);
    exit(1);
  }

  ar->elems[idx] = elem;
}
