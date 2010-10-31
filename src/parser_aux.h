#ifndef VMSCHEME_PARSER_AUX
#define VMSCHEME_PARSER_AUX

#include <object.h>

struct pair* parse_interactive(void);
struct pair* parse_file(const char *filename);

#endif // VMSCHEME_PARSER_AUX
