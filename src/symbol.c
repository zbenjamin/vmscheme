#include <symbol.h>
#include <object.h>

#include <string.h>

void
init_symbol_table()
{
  array_create(&symbol_table, sizeof(struct object *));
}

struct object *
get_symbol(char *name)
{
  struct object *sym;
  int i;
  int size = array_size(&symbol_table);
  for (i = 0; i < size; ++i) {
    sym = *obj_array_ref(&symbol_table, i);
    if (strcmp(name, sym->sval) == 0) {
      return sym;
    }
  }

  sym = make_symbol(strdup(name));
  INC_REF(sym);
  array_add(&symbol_table, &sym);
  return sym;
}
