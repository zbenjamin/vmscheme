#include <type.h>

struct type *nil_type;
struct type *integer_type;
struct type *string_type;
struct type *symbol_type;
struct type *pair_type;
struct type *procedure_type;
struct type *primitive_proc_type;

void
init_builtin_types()
{
  nil_type = make_type("nil");
  integer_type = make_type("integer");
  string_type = make_type("string");
  symbol_type = make_type("symbol");
  pair_type = make_type("pair");
  procedure_type = make_type("procedure");
  primitive_proc_type = make_type("primitive-procedure");
}
