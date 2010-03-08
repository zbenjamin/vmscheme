#ifndef VMSCHEME_PARSER_AUX
#define VMSCHEME_PARSER_AUX

#include <object.h>

struct object* parse_interactive();
struct object* parse_file(const char *filename);

#endif // VMSCHEME_PARSER_AUX
