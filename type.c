#include <string.h>
#include <type.h>

#include <stdlib.h>

#define MAX_TYPES 100

static struct type type_table[MAX_TYPES];
static unsigned int next_entry = 0;

struct type*
get_type(char* name)
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
