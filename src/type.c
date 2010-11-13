#include <string.h>
#include <type.h>

#include <stdlib.h>

#define MAX_TYPES 100

static struct type type_table[MAX_TYPES];
static unsigned int next_entry = 0;

void
init_builtin_types(void)
{
  make_type("nil");
  make_type("unspecific");
  make_type("boolean");
  make_type("integer");
  make_type("string");
  make_type("symbol");
  make_type("pair");
  make_type("procedure");
  make_type("code");
  make_type("code-pointer");
  make_type("environment");
  make_type("continuation");
}

struct type*
get_type(int typeid)
{
  if (typeid < next_entry) {
    return &type_table[typeid];
  }
  return NULL;
}

struct type*
get_type_named(char* name)
{
  int i;
  for (i = 0; i < MAX_TYPES; ++i) {
    if (strcmp(name, type_table[i].name)) {
      return &type_table[i];
    }
  }
  return NULL;
}

struct type*
make_type(char* name)
{
  type_table[next_entry].code = next_entry;
  type_table[next_entry].name = strdup(name);
  
  return &type_table[next_entry++];
}
