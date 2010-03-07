#include <parser_aux.h>

#include <parser.h>

#include <stdio.h>

extern FILE *yyin;
void reset_scanner_start_condition();
int yyparse(void*, struct object** parse_result);

struct object*
parse_interactive()
{
  struct object *result;
  yyin = stdin;
  reset_scanner_start_condition();
  yyparse(stdin, &result);
  return result;
}
