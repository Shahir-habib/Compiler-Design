#@title Writing Lex program
%%writefile program.l

%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int num1,num2;
char operator;
%}

%%
[0-9]+[+\-*/%^][0-9]+ {

          int i=0;
          char s[100];
          strcpy(s,yytext);
          while(i<strlen(s)){
            num1=(num1*10)+(s[i]-'0');
            i++;
          }

          operator =  s[i];
          while(i<strlen(s)){
            num2=(num2*10)+(s[i]-'0');
            i++;
          }
           switch(operator) {
               case '+': printf("Result: %d\n", num1 + num2); break;
               case '-': printf("Result: %d\n", num1 - num2); break;
               case '*': printf("Result: %d\n", num1 * num2); break;
               case '/': 
                   if (num2 != 0)
                       printf("Result: %.2f\n", (double)num1 / num2);
                   else
                       printf("Error: Division by zero!\n");
                   break;
               case '%': printf("Result: %d\n", num1 % num2); break;
               case '^': printf("Result: %.2f\n", pow(num1, num2)); break;
               default: printf("Invalid operator!\n");
           }
          }
.|\n      { printf("Invalid input!\n"); }
%%

int main() {
    printf("Enter your expression (e.g., 5 + 3): ");
    yylex();  // Start lexical analysis
    return 0;
}


int yywrap(){
    return 1;
}