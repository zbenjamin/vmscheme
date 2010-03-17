#include <parser_aux.h>

#include <parser.h>

#include <stdio.h>

extern FILE *yyin;
void set_scanner_interactive(int flag);
void reset_scanner();
int yyparse(void*, struct object** parse_result);
extern int yydebug;

struct object*
parse_interactive()
{
  struct object *result;
  yyin = stdin;
  set_scanner_interactive(1);
  reset_scanner();
  yyparse(stdin, &result);
  return result;
}

struct object*
parse_file(const char *filename)
{
  struct object *result;
  FILE *fh = fopen(filename, "r");
  yyin = fh;
  set_scanner_interactive(0);
  reset_scanner();
  yyparse(fh, &result);
  fclose(fh);
  return result;
}
