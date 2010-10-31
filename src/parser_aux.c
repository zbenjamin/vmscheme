#include <debug.h>
#include <parser_aux.h>

#include <parser.h>

#include <malloc.h>
#include <stdio.h>
#include <string.h>

extern FILE *yyin;
void set_scanner_interactive(int flag);
void reset_scanner(void);
int yyparse(void*, struct pair** parse_result,
            struct debuginfo *dbiproto);
extern int yydebug;

struct pair*
parse_interactive(void)
{
  struct pair *result;
  struct debuginfo dbiproto;
  dbiproto.src = SRC_STDIN;
  dbiproto.src_file = NULL;

  yyin = stdin;
  set_scanner_interactive(1);
  reset_scanner();
  yyparse(stdin, &result, &dbiproto);
  return result;
}

struct pair*
parse_file(const char *filename)
{
  struct pair *result;
  struct debuginfo dbiproto;
  dbiproto.src = SRC_FILE;
  dbiproto.src_file = strdup(filename);

  FILE *fh = fopen(filename, "r");
  yyin = fh;
  set_scanner_interactive(0);
  reset_scanner();
  yyparse(fh, &result, &dbiproto);
  fclose(fh);
  free(dbiproto.src_file);
  return result;
}
