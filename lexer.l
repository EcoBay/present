%{
#include "present.tab.h"
#include "present.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
%}

%option yylineno nodefault noyywrap

%%

"up"    |
"down"  |
"left"  |
"right"     { strcpy(yylval.s, yytext); return DIRECTION; }

"box"       |
"circle"    |
"ellipse"   |
"arc"       |
"line"      |
"arrow"     |
"spline"        { strcpy(yylval.s, yytext); return PRIMITIVE; }

"\\#".*
[ \t]
\n          { return EOL; }

.           {
                yyerror(
                    "Unexpected character %c at line no. %d\n",
                    *yytext, yylineno
                );
                exit(EXIT_FAILURE);
            }
%%

void yyerror(char *s, ...){
    if(!strcmp(s, "syntax error")){
        fprintf(stderr, "Syntax error at line no. %d\n", yylineno);
        exit(EXIT_FAILURE);
    }else{
        va_list ap;

        va_start(ap, s);
        vfprintf(stderr, s, ap);
        va_end(ap);
    }
}
