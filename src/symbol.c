#include <symbol.h>
#include <object.h>

#include <string.h>

void
init_symbol_table(void)
{
  array_create(&symbol_table, sizeof(struct symbol *));
}

struct symbol *
get_symbol(char *name)
{
  struct symbol *sym;
  int i;
  int size = array_size(&symbol_table);
  for (i = 0; i < size; ++i) {
    sym = *(struct symbol**) array_ref(&symbol_table, i);
    if (strcmp(name, sym->value) == 0) {
      return sym;
    }
  }

  sym = make_symbol(strdup(name));
  INC_REF(&sym->obj);
  array_add(&symbol_table, &sym);
  return sym;
}
