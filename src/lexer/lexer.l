%{
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>

int lexical_error_count = 0;


#define TOK(name) do { \
    printf("  line %-4d  %-12s  %s\n", yylineno, #name, yytext); \
    return name; \
} while (0)
%}

%option yylineno
%option noyywrap
%option noinput
%option nounput

%x COMMENT

DIGIT    [0-9]
LETTER   [a-zA-Z_]
ID       {LETTER}({LETTER}|{DIGIT})*
INTLIT   {DIGIT}+
FLOATLIT {DIGIT}+"."{DIGIT}+

%%


"//".*                  { /* single-line comment: discard */ }

"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>\n             { /* yylineno auto-increments (yylineno option) */ }
<COMMENT>.              { /* discard comment body char-by-char */ }


[ \t\r]+                { /* discard spaces/tabs */ }
\n                       { /* discard newline */ }



"int"                    { TOK(INT); }
"float"                  { TOK(FLOAT); }
"bool"                   { TOK(BOOL); }
"if"                     { TOK(IF); }
"else"                   { TOK(ELSE); }
"while"                  { TOK(WHILE); }
"print"                  { TOK(PRINT); }
"true"                   { yylval.ival = 1; TOK(TRUE); }
"false"                  { yylval.ival = 0; TOK(FALSE); }


{FLOATLIT}               { yylval.fval = (float)atof(yytext); TOK(FLOAT_LIT); }
{INTLIT}                 { yylval.ival = atoi(yytext);        TOK(INT_LIT); }

    /* -------------------- Identifiers -------------------- */
{ID}                     { yylval.sval = strdup(yytext); TOK(IDENTIFIER); }

    /* -------------------- Arithmetic operators -------------------- */
"+"                      { TOK(PLUS); }
"-"                      { TOK(MINUS); }
"*"                      { TOK(MUL); }
"/"                      { TOK(DIV); }
"%"                      { TOK(MOD); }

    /* ---------- Relational operators (2-char before 1-char) ---------- */
"<="                     { TOK(LE); }
">="                     { TOK(GE); }
"=="                     { TOK(EQ); }
"!="                     { TOK(NE); }
"<"                      { TOK(LT); }
">"                      { TOK(GT); }

    /* -------------------- Logical operators -------------------- */
"&&"                     { TOK(AND); }
"||"                     { TOK(OR); }
"!"                      { TOK(NOT); }

    /* -------------------- Assignment -------------------- */
"="                      { TOK(ASSIGN); }

    /* -------------------- Delimiters -------------------- */
"{"                      { TOK(LBRACE); }
"}"                      { TOK(RBRACE); }
"("                      { TOK(LPAREN); }
")"                      { TOK(RPAREN); }
";"                      { TOK(SEMI); }

    /* -------------------- Invalid tokens -------------------- */
.                        {
                            fprintf(stderr,
                                "Lexical Error at line %d: unexpected character '%s'\n",
                                yylineno, yytext);
                            lexical_error_count++;
                         }

%%
