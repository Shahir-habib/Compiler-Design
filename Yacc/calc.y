%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.yy.h"  // This should be included to access Flex's functions and types.

void yyerror(const char *s) {
    printf("Invalid Input\n");
}

int yylex(void);
%}

%token INTEGER NL
%left '+' '-'
%left '*' '/'
%left '%'

%%

program: 
        program expr NL         { printf("Result : %d\n", $2); } 
        |  
        ;
expr: INTEGER { $$ = $1; }
    | expr '+' expr { $$ = $1 + $3; }
    | expr '-' expr { $$ = $1 - $3; }
    | expr '*' expr { $$ = $1 * $3; }
    | expr '/' expr { $$ = $1 / $3; }
    | expr '%' expr { $$ = $1 % $3; }
    | '(' expr ')' { $$ = $2; }
    ;
%%

int main(void) {
    char line[100]; 
    printf("Enter the Input :\n");

    while (1) {
        if (!fgets(line, sizeof(line), stdin)) {
            break; // Exit the loop if there's no more input
        }

        YY_BUFFER_STATE buffer = yy_scan_string(line);
        int result = yyparse();  
        if (result != 0) {  
            printf("Some Error while calculating.\n");
        }

        yy_delete_buffer(buffer);

        printf("Enter Input:\n");  
    }

    return 0;
}