#include <type.h>

void
init_builtin_types()
{
  make_type("nil");
  make_type("integer");
  make_type("string");
  make_type("symbol");
  make_type("pair");
  make_type("procedure");
  make_type("primitive-procedure");
  make_type("code");
  make_type("environment");
}
