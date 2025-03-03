%{
#include "calc.tab.h"
%}

%%
[0-9]+      {   yylval = atoi(yytext); 
                return INTEGER; 
            }
\n {return NL;}
[-+()/*%]      return *yytext; 
[ \t]     
; /* skip whitespace */ 
%%

int yywrap() {
    return 1;
}
