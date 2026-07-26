%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern FILE *yyin;
extern int lexical_error_count;   /* defined in lexer.l */
%}


%union {
    int   ival;
    float fval;
    char  *sval;
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

%start program

%%

 
program:
      /* empty */
    | program token
    ;

token:
      INT | FLOAT | BOOL
    | IF | ELSE | WHILE | PRINT
    | TRUE | FALSE
    | IDENTIFIER | INT_LIT | FLOAT_LIT
    | PLUS | MINUS | MUL | DIV | MOD
    | LT | GT | LE | GE | EQ | NE
    | AND | OR | NOT
    | ASSIGN
    | LBRACE | RBRACE | LPAREN | RPAREN | SEMI
    ;

%%

void yyerror(const char *s) {
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
    if (result != 0) {
        fprintf(stderr, "\nCompilation failed: syntax error(s) found.\n");
        return 1;
    }

    printf("\n[Day 1 wiring test] SUCCESS — lexer and parser are correctly "
           "wired. Every token in '%s' was recognized and matched the "
           "grammar's token set.\n", argv[1]);
    return 0;
}
