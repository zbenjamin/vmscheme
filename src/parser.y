%defines
%define api.pure
%locations

%{
#include <debug.h>
#include <string.h>
#include <object.h>
#include <primitive_procedures.h>
#include <symbol.h>
#include <utils.h>
%}

%union {
  int num;
  char *str;
  struct object *obj;
  struct pair *pair;
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
%token <num> INTERACTIVE_EOF

%type <pair> prog
%type <obj> expr
%type <pair> exprseq

%start prog

%{
#include <stdio.h>
#include <stdlib.h>

typedef void* yyscan_t;

int yylex(YYSTYPE *lvalp, YYLTYPE *llocp, yyscan_t scanner);

void yyerror(YYLTYPE *locp, yyscan_t scanner,
             struct pair** parse_result,
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

%parse-param {struct pair **parse_result}
%parse-param {struct debuginfo *dbiproto}

%%

prog: /* empty */ { *parse_result = NIL; }
    | INTERACTIVE_EOF { *parse_result = NULL; YYACCEPT; }
    | exprseq { *parse_result = reverse_list($$);
                 dealloc_obj(&$$->obj);
                 set_debug_info(&(*parse_result)->obj,
                                loc2dbi(dbiproto, &@$));
      }
  ;

expr: NUMBER { $$ = &make_integer($1)->obj;
               set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | SYMBOL { $$ = &get_symbol($1)->obj;
               set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | STRING { $$ = &make_string($1)->obj;
               set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | BOOL_TRUE { $$ = TRUE; }
    | BOOL_FALSE { $$ = FALSE; }
    | QUOTE expr { $$ = &make_pair(&get_symbol("quote")->obj,
                                   &make_pair($2,
                                              &NIL->obj)->obj)->obj;
                   set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | QUASIQUOTE expr
      { $$ = &make_pair(&get_symbol("quasiquote")->obj,
                        &make_pair($2, &NIL->obj)->obj)->obj;
        set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | UNQUOTE expr { $$ = &make_pair(&get_symbol("unquote")->obj,
                                     &make_pair($2,
                                                &NIL->obj)->obj)->obj;
                     set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | UNQUOTE_SPLICING expr
      { $$ = &make_pair(&get_symbol("unquote-splicing")->obj,
                        &make_pair($2, &NIL->obj)->obj)->obj;
        set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | LP exprseq RP { $$ = &reverse_list($2)->obj;
                      dealloc_obj(&$2->obj);
                      set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | LP exprseq DOT expr RP
      { $$ = &reverse_list($2)->obj;
        dealloc_obj(&$2->obj);
        set_cdr(last_pair($$), $4);
        set_debug_info($$, loc2dbi(dbiproto, &@$)); }
    | LP RP { $$ = &NIL->obj; }
  ;

exprseq: expr         { $$ = make_pair($1, &NIL->obj); }
       | exprseq expr { $$ = make_pair($2, &$1->obj); }
  ;
