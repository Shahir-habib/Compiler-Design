%{
    #include <stdio.h>
%}

%%
1(0|1|.)*101|101 {printf("Input : %s Passed\n",yytext);}
.* {printf("Input : %s Failed \n",yytext);}
%%



void main(int argc, char *argv[]){
    
    yylex();


}

int yywrap(){
    return 1;
}