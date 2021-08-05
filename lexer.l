%{
#include "present.tab.h"
#include "present.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
%}

%option yylineno nodefault noyywrap

%%

"box"       { return BOX; }
"circle"    { return CIRCLE; }
"ellipse"   { return ELLIPSE; }
"arc"       { return ARC; }
"line"      { return LINE; }
"arrow"     { return ARROW; }
"spline"    { return SPLINE; }
"move"      { return MOVE; }
\"([^\\\"]|\\.)*\" {
                yytext[yyleng - 1] = '\0';
                yylval.s = strdup(yytext + 1);
                return TEXT;
            }

"up"        { return UP; }
"down"      { return DOWN; }
"left"      { return LEFT; }
"right"     { return RIGHT; }
"then"      { return THEN; }

"<-"        { return LARROW; }
"->"        { return RARROW; }
"<->"       { return LRARROW; }

"cw"        { return CW; }

"\\#".*
[ \t]
\n          { return EOL; }
";"         { return *yytext; }

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
