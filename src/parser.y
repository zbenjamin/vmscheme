%defines
%define api.pure
%locations

%{
#include <debug.h>
#include <string.h>
#include <object.h>
#include <primitive_procedures.h>
#include <symbol.h>
%}

%union {
  int num;
  char *str;
  struct object *obj;
}

%token <num> LP
%token <num> RP
%token <num> NUMBER
%token <str> SYMBOL
%token <str> STRING
%token <num> BOOL_TRUE
%token <num> BOOL_FALSE
%token <num> QUOTE
%token <num> QUASIQUOTE
%token <num> UNQUOTE
%token <num> UNQUOTE_SPLICING
%token <num> DOT

%type <obj> prog
%type <obj> expr
%type <obj> exprseq

%start prog

%{
#include <stdio.h>
#include <stdlib.h>

typedef void* yyscan_t;

int yylex(YYSTYPE *lvalp, YYLTYPE *llocp, yyscan_t scanner);

void yyerror(YYLTYPE *locp, yyscan_t scanner,
             struct object** parse_result,
             struct debuginfo *dbiproto,
             const char *s) {
  printf("ERROR: %s\n", s);
  exit(1);
}

struct debuginfo *
loc2dbi(struct debuginfo *proto, YYLTYPE *loc)
{
  struct debuginfo *dbi = malloc(sizeof(struct debuginfo));
  memcpy(dbi, proto, sizeof(struct debuginfo));
  if (proto->src_file) {
    // XXX: this is really inefficient
    dbi->src_file = strdup(proto->src_file);
  }
  dbi->first_line = loc->first_line;
  dbi->first_column = loc->first_column;
  dbi->last_line = loc->last_line;
  dbi->last_column = loc->last_column;
  return dbi;
}

%}

/* Pass the argument to yyparse through to yylex. */
%parse-param {yyscan_t scanner}
%lex-param {yyscan_t scanner}

%parse-param {struct object **parse_result}
%parse-param {struct debuginfo *dbiproto}

%%

prog: /* empty */ { *parse_result = NULL; }
    | exprseq { *parse_result = reverse_list($$);
                 dealloc_obj($$);
                 set_debug_info($$, loc2dbi(dbiproto, &@$));
      }
  ;

expr: NUMBER { $$ = make_integer($1);
               set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | SYMBOL { $$ = get_symbol($1);
               set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | STRING { $$ = make_string($1);
               set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | BOOL_TRUE { $$ = TRUE; }
    | BOOL_FALSE { $$ = FALSE; }
    | QUOTE expr { $$ = make_pair(get_symbol("quote"),
                                  make_pair($2, NIL));
                   set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | QUASIQUOTE expr
      { $$ = make_pair(get_symbol("quasiquote"),
                       make_pair($2, NIL));
        set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | UNQUOTE expr { $$ = make_pair(get_symbol("unquote"),
                                    make_pair($2, NIL));
                     set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | UNQUOTE_SPLICING expr
      { $$ = make_pair(get_symbol("unquote-splicing"),
                       make_pair($2, NIL));
        set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | LP exprseq RP { $$ = reverse_list($2);
                      dealloc_obj($2);
                      set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | LP exprseq DOT expr RP
      { $$ = reverse_list($2);
        dealloc_obj($2);
        set_cdr(last_pair($$), $4);
        set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | LP RP { $$ = NIL; }
  ;

exprseq: expr         { $$ = make_pair($1, NIL); }
       | exprseq expr { $$ = make_pair($2, $1); }
  ;
