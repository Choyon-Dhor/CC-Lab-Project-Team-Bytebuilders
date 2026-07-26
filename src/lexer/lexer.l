%{
#include <stdio.h>
%}

%%

.   { printf("%s", yytext); }

%%

int yywrap() {
    return 1;
}