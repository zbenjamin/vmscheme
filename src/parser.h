#ifndef VMSCHEME_PARSER_H
#define VMSCHEME_PARSER_H

struct object* parse(char *str);
struct object* parse_file(const char *filename);
#endif // VMSCHEME_PARSER_H
