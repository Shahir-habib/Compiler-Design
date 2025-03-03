%{
    #include <stdio.h>
%}

%%

^[A-Z][a-z0-9]*[!@#$%^]$ {
    printf("Input : %s Passed\n",yytext);
}

.* {printf("Input : %s Failed \n",yytext);}

%%

int main() {
    yylex();
    return 0;
}
int yywrap(){
    return 1;
}