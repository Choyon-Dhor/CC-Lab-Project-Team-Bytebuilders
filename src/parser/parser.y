%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "semantic_analyzer.h"

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern FILE *yyin;
extern int lexical_error_count;

int syntax_error_count = 0;

ASTNode *ast_root = NULL;
%}

%union {
    int   ival;
    float fval;
    char  *sval;
    struct ASTNode *node;
}


%token INT FLOAT BOOL
%token IF ELSE WHILE PRINT
%token <ival> TRUE FALSE


%token <sval> IDENTIFIER
%token <ival> INT_LIT
%token <fval> FLOAT_LIT


%token PLUS MINUS MUL DIV MOD


%token LT GT LE GE EQ NE


%token AND OR NOT


%token ASSIGN

%token LBRACE RBRACE LPAREN RPAREN SEMI

%type <node> program stmt_list stmt decl_stmt assign_stmt
%type <node> if_stmt while_stmt print_stmt block expr
%type <sval> type


%left OR
%left AND
%precedence NOT
%nonassoc LT GT LE GE EQ NE
%left PLUS MINUS
%left MUL DIV MOD
%precedence UMINUS

%start program

%%

program:
      stmt_list { ast_root = $1; $$ = $1; }
    ;

stmt_list:
      %empty              { $$ = NULL; }
    | stmt_list stmt       { $$ = $2 ? ast_append_stmt($1, $2) : $1; }
    ;

stmt:
      decl_stmt
    | assign_stmt
    | if_stmt
    | while_stmt
    | print_stmt
    | block
    | error SEMI { yyerrok; $$ = NULL; }
    ;

type:
      INT    { $$ = "int"; }
    | FLOAT  { $$ = "float"; }
    | BOOL   { $$ = "bool"; }
    ;

decl_stmt:
      type IDENTIFIER SEMI { $$ = ast_make_decl($1, $2, yylineno); }
    ;

assign_stmt:
      IDENTIFIER ASSIGN expr SEMI { $$ = ast_make_assign($1, $3, yylineno); }
    ;

if_stmt:
      IF LPAREN expr RPAREN block
          { $$ = ast_make_if($3, $5, NULL, yylineno); }
    | IF LPAREN expr RPAREN block ELSE block
          { $$ = ast_make_if($3, $5, $7, yylineno); }
    ;

while_stmt:
      WHILE LPAREN expr RPAREN block { $$ = ast_make_while($3, $5, yylineno); }
    ;

print_stmt:
      PRINT expr SEMI { $$ = ast_make_print($2, yylineno); }
    ;

block:
      LBRACE stmt_list RBRACE { $$ = ast_make_block($2, yylineno); }
    ;

expr:
      expr OR expr    { $$ = ast_make_binop("||", $1, $3, yylineno); }
    | expr AND expr   { $$ = ast_make_binop("&&", $1, $3, yylineno); }
    | NOT expr        { $$ = ast_make_unop("!", $2, yylineno); }
    | expr LT expr    { $$ = ast_make_binop("<",  $1, $3, yylineno); }
    | expr GT expr    { $$ = ast_make_binop(">",  $1, $3, yylineno); }
    | expr LE expr    { $$ = ast_make_binop("<=", $1, $3, yylineno); }
    | expr GE expr    { $$ = ast_make_binop(">=", $1, $3, yylineno); }
    | expr EQ expr    { $$ = ast_make_binop("==", $1, $3, yylineno); }
    | expr NE expr    { $$ = ast_make_binop("!=", $1, $3, yylineno); }
    | expr PLUS expr  { $$ = ast_make_binop("+",  $1, $3, yylineno); }
    | expr MINUS expr { $$ = ast_make_binop("-",  $1, $3, yylineno); }
    | expr MUL expr   { $$ = ast_make_binop("*",  $1, $3, yylineno); }
    | expr DIV expr   { $$ = ast_make_binop("/",  $1, $3, yylineno); }
    | expr MOD expr   { $$ = ast_make_binop("%",  $1, $3, yylineno); }
    | MINUS expr %prec UMINUS { $$ = ast_make_unop("-", $2, yylineno); }
    | LPAREN expr RPAREN      { $$ = $2; }
    | IDENTIFIER      { $$ = ast_make_identifier($1, yylineno); }
    | INT_LIT         { $$ = ast_make_int_lit($1, yylineno); }
    | FLOAT_LIT       { $$ = ast_make_float_lit($1, yylineno); }
    | TRUE            { $$ = ast_make_bool_lit(1, yylineno); }
    | FALSE           { $$ = ast_make_bool_lit(0, yylineno); }
    ;

%%

void yyerror(const char *s) {
    syntax_error_count++;
    fprintf(stderr, "Syntax Error at line %d: %s\n", yylineno, s);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
            return 1;
        }
        yyin = f;
    } else {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    printf("----- TOKEN STREAM -----\n");
    int result = yyparse();

    if (lexical_error_count > 0) {
        fprintf(stderr, "\nCompilation failed: %d lexical error(s) found.\n",
                lexical_error_count);
        return 1;
    }
    if (result != 0 || syntax_error_count > 0) {
        fprintf(stderr, "\nCompilation failed: %d syntax error(s) found.\n",
                syntax_error_count);
        return 1;
    }

    printf("\n----- ABSTRACT SYNTAX TREE -----\n");
    ast_print_list(ast_root, 0);

    int semantic_errors = semantic_analyze(ast_root);
    if (semantic_errors > 0) {
        fprintf(stderr, "\nCompilation failed: %d semantic error(s) found.\n", semantic_errors);
        return 1;
    }

    printf("\nSUCCESS — parsed and semantically validated.\n");
    return 0;
}
