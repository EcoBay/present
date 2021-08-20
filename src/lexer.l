%{
#include "present.h"
#include "symtable.h"
#include "present.tab.h"
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

"for"       { return FOR; }
"of"        { return OF; }
"here"      { return HERE; }
"and"       { return AND; }
"between"   { return BETWEEN; }

"up"        { return UP; }
"down"      { return DOWN; }
"left"      { return LEFT; }
"right"     { return RIGHT; }
"by"        { return BY; }
"to"        { return TO; }
"from"      { return FROM; }
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

".n"        { return DOT_N; }
".e"        { return DOT_E; }
".w"        { return DOT_W; }
".s"        { return DOT_S; }
".c"        { return DOT_C; }
".ne"       { return DOT_NE; }
".nw"       { return DOT_NW; }
".se"       { return DOT_SE; }
".sw"       { return DOT_SW; }
".start"    { return DOT_START; }
".end"      { return DOT_END; }

"top"       { return TOP; }
"bot"       { return BOT; }
"start"     { return START; }
"end"       { return END; }

"at"        { return AT; }
"with"      { return WITH; }

"ht"        |
"height"    { return HT; }
"wid"       |
"width"     { return WID; }
"rad"       |
"radius"    { return RAD; }
"diam"      |
"daimeter"  { return DIAM; }
"chop"      { return CHOP; }

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

"keyframe"  { return KEYFRAME; }
"scene"     { return SCENE; }

"linear"    { return LINEAR; }
"sine"      { return SINE; }
"quadratic" { return QUADRATIC; }
"cubic"     { return CUBIC; }

"in"        { return IN; }
"out"       { return OUT; }

"s"         |
"second"s?  { return SECONDS; }
"ms"            |
"milliseconds"  { return MILLISECONDS; }
"m"         |
"minute"s?  { return MINUTES; }

[a-zA-Z_][a-zA-Z0-9_]* {
                yylval.s = strdup(yytext);
                struct symbol *s;
                s = lookup(yytext);
                if (!s || s -> t == SYM_DOUBLE) {
                    return IDENTIFIER;
                } else {
                    return LABEL;
                }
            }
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
"["         |
"]"         |
"="         |
":"         |
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
