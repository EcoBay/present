%{
#include "present.tab.h"
#include "present.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

"up"        { return UP; }
"down"      { return DOWN; }
"left"      { return LEFT; }
"right"     { return RIGHT; }
"by"        { return BY; }
"then"      { return THEN; }

"<-"        { return LARROW; }
"->"        { return RARROW; }
"<->"       { return LRARROW; }

"cw"        { return CW; }

"dashed"    { return DASHED; }
"dotted"    { return DOTTED; }
"invis"     { return INVIS; }
"solid"     { return SOLID; }
"fill"      { return FILL; }

"center"    { return CENTER; }
"ljust"     { return LJUST; }
"rjust"     { return RJUST; }
"above"     { return ABOVE; }
"below"     { return BELOW; }

"ht"        |
"height"    { return HT; }
"wid"       |
"width"     { return WID; }
"rad"       |
"radius"    { return RAD; }
"diam"      |
"daimeter"  { return DIAM; }

([0-9]*[.])?[0-9]+([eE][-+]?[0-9]+)? {
                yylval.d = strtof(yytext, NULL);
                return NUMBER;
            }
\"([^\\\"]|\\.)*\" {
                yytext[yyleng - 1] = '\0';
                yylval.s = strdup(yytext + 1);
                return TEXT;
            }
#[0-9a-fA-F]{8} |
#[0-9a-fA-F]{6} |
#[0-9a-fA-F]{4} |
#[0-9a-fA-F]{3} {
                yylval.s = strdup(yytext + 1);
                return HEXCOLOR;
            }

"sin"       { return SIN; }
"cos"       { return COS; }
"atan2"     { return ATAN2; }
"log"       { return LOG; }
"exp"       { return EXP; }
"sqrt"      { return SQRT; }
"max"       { return MAX; }
"min"       { return MIN; }
"int"       { return INT; }
"rand"      { return RAND; }
"abs"       { return ABS; }
"rgba"      { return RGBA; }

"\\#".*
[ \t]
\n          { return EOL; }
";"         |
"+"         |
"-"         |
"*"         |
"/"         |
"%"         |
"!"         |
"^"         |
"("         |
")"         |
","         { return *yytext; }

.           {
                yyerror(
                    "Unexpected character %c at line no. %d\n",
                    *yytext, yylineno
                );
                abort();
            }
%%

void yyerror(char *s, ...){
    if(!strcmp(s, "syntax error")){
        fprintf(stderr, "Syntax error at line no. %d\n", yylineno);
        abort();
    }else{
        va_list ap;

        va_start(ap, s);
        vfprintf(stderr, s, ap);
        va_end(ap);
    }
}
