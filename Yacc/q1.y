%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.yy.h"  // This should be included to access Flex's functions and types.

void yyerror(const char *s) {
    printf("Invalid string\n");
}

int yylex(void);
%}

%token A B NL

%%

input: expr NL { printf("Valid string\n"); }
     ;

expr: A expr B
    | /* empty */  // Empty production to allow an empty expression.
    ;

%%

int main(void) {
    char line[100]; 
    printf("Enter string:\n");

    while (1) {
        if (!fgets(line, sizeof(line), stdin)) {
            break; // Exit the loop if there's no more input
        }

        YY_BUFFER_STATE buffer = yy_scan_string(line);
        int result = yyparse();  
        if (result != 0) {  
            printf("No of A's and B's not equal or B don't follow A.\n");
        }

        yy_delete_buffer(buffer);

        printf("Enter string:\n");  
    }

    return 0;
}