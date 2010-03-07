%defines
%define api.pure

%{
#include <object.h>
#include <primitive_procedures.h>
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
%token <num> BOOL_TRUE
%token <num> BOOL_FALSE
%token <num> QUOTE

%type <obj> prog
%type <obj> expr
%type <obj> exprseq

%start prog
%{
#include <stdio.h>
#include <stdlib.h>

typedef void* yyscan_t;

int yylex(YYSTYPE *lvalp, yyscan_t scanner);
void yyerror(yyscan_t scanner, struct object** parse_result,
             const char *s);

void yyerror(yyscan_t scanner, struct object** parse_result,
             const char *s) {
  printf("ERROR: %s\n", s);
  exit(1);
}

%}

/* Pass the argument to yyparse through to yylex. */
%parse-param {yyscan_t scanner}
%lex-param {yyscan_t scanner}

%parse-param {struct object **parse_result}

%%

prog: /* empty */ { *parse_result = NULL; }
    | exprseq { *parse_result = reverse_list($$); }
  ;

expr: NUMBER { $$ = make_integer($1); }
    | SYMBOL { $$ = make_symbol($1); }
    | BOOL_TRUE { $$ = TRUE; }
    | BOOL_FALSE { $$ = FALSE; }
    | QUOTE expr { $$ = make_pair(make_symbol("quote"),
                                  make_pair($2, NIL)); }
    | LP exprseq RP { $$ = reverse_list($2); }
  ;

exprseq: expr         { $$ = make_pair($1, NIL); }
       | exprseq expr { $$ = make_pair($2, $1); }
  ;
