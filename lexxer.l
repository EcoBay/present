%{
#include "present.tab.h"
#include "present.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
%}

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
.           { yyerror("Unexpected character %c\n", *yytext); }
%%

void yyerror(char *s, ...){
    va_list ap;

    va_start(ap, s);
    vfprintf(stderr, s, ap);
    va_end(ap);
}
