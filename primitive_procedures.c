#include <type.h>

#include <object.h>

#include <stdio.h>
#include <stdlib.h>

struct object*
car(struct object *pair)
{
  if (pair->type != pair_type) {
    printf("Wrong type for car: %s\n", pair->type->name);
    exit(1);
  }
}
