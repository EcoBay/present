%{
#include "present.h"
#include "symtable.h"
#include "present.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define yyterminate() freeTable(popTable()); return YY_NULL
#define YY_NO_INPUT
#define YY_NO_UNPUT

int G_ARGCTR;
char *G_IDEN;
char *G_ARGS[10];

char* str_replace(char*, char*, char*);
void expand_macro();
%}

%option yylineno nodefault noyywrap
%x MACRO_CALL MACRO_ARG_0 MACRO_ARG_1
%s MACRO_DEF

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
"define"    { BEGIN MACRO_DEF; return DEFINE; }
"reset"     { return RESET; }
"print"     { return PRINT; }
"last"      { return LAST; }
"block"     { return BLOCK; }
"text"      { return TEXT_L; }
"do"        { return DO; }
[xX]        |
"times"     { return TIMES; }
"if"        { return IF; }
"else"      { return ELSE; }
"sprintf"   { return SPRINTF; }

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
"bot"       |
"bottom"    { return BOT; }
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
"same"      { return SAME; }
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

"=="        { return EE; }
"!="        { return NE; }
">="        { return GE; }
"<="        { return LE; }
"||"        { return OROR; }
"&&"        { return ANDAND; }

[a-z][a-zA-Z0-9_]*  {
                struct symbol *s;
                s = lookup(yytext);
                if (!s || s -> t == SYM_DOUBLE) {
                    yylval.s = strdup(yytext);
                    return IDENTIFIER;
                } else {
                    G_IDEN = strdup(yytext);
                    BEGIN MACRO_CALL;
                }
            }
[A-Z][a-zA-Z0-9_]*  {
                struct symbol *s;
                s = lookup(yytext);
                if (!s || s -> t == SYM_EVENT) {
                    yylval.s = strdup(yytext);
                    return LABEL;
                } else {
                    G_IDEN = strdup(yytext);
                    BEGIN MACRO_CALL;
                }
            }

<MACRO_CALL>[ \t\n]     {
                for (int i = 0; i < 10; i++) {
                    G_ARGS[i] = "";
                }
                expand_macro();
                BEGIN INITIAL;
            }
    /* TODO: Allow for balanced parenthesis */
<MACRO_CALL>"("         {
                for (int i = 0; i < 10; i++) {
                    G_ARGS[i] = "";
                }
                
                G_ARGCTR = 0;
                BEGIN MACRO_ARG_0;
            }
<MACRO_CALL>"{"         {
                for (int i = 0; i < 10; i++) {
                    G_ARGS[i] = "";
                }
                
                G_ARGCTR = 0;
                BEGIN MACRO_ARG_1;
            }
<MACRO_ARG_0>[^,)]*","  |
<MACRO_ARG_1>[^,}]*","  { 
                yytext[yyleng - 1] = '\0';
                G_ARGS[G_ARGCTR++] = strdup(yytext);
                if (G_ARGCTR > 9) {
                    yyerror("Macro arguments count exceeds 10\n");
                    abort();
                }
            }
<MACRO_ARG_0>[^,)]*")"   | 
<MACRO_ARG_1>[^,}]*"}"   { 
                yytext[yyleng - 1] = '\0';
                G_ARGS[G_ARGCTR++] = strdup(yytext);
                if (G_ARGCTR > 9) {
                    yyerror("Macro arguments count exceeds 10\n");
                    abort();
                }
                expand_macro();
                BEGIN INITIAL;
            }

<MACRO_ARG_0><<EOF>>    |
<MACRO_ARG_1><<EOF>>    |
<MACRO_CALL><<EOF>>     {
                yyerror("Macro call not closed\n");
                abort();
            }

<MACRO_DEF>"X"[^X]*"X"      |
<MACRO_DEF>"%"[^%]*"%"      |
<MACRO_DEF>"["[^\]]*"]"     |
<MACRO_DEF>"{"[^}]*"}"      {
                yytext[yyleng - 1] = '\0';
                yylval.s = strdup(yytext + 1);
                BEGIN INITIAL;
                return TEMPLATE;
            }
([0-9]*[.])?[0-9]+([eE][-+]?[0-9]+)? {
                yylval.d = strtof(yytext, NULL);
                return NUMBER;
            }
[0-9]*(1st|2nd|3rd|[0-9]th) {
                yytext[yyleng - 2] = '\0';
                yylval.i = atoi(yytext);
                return ORDINAL;
            }
\"([^\\\"]|\\.)*\"  {
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
"#".*
[ \t]
\n          { return EOL; }
";"         |
"+"         |
"-"         |
"*"         |
"/"         |
"%"         |
"!"         |
"<"         |
">"         |
"^"         |
"("         |
")"         |
"["         |
"]"         |
"{"         |
"}"         |
"="         |
":"         |
"."         |
","         { return *yytext; }

<<EOF>>     {
                yypop_buffer_state();
                if (!YY_CURRENT_BUFFER) {
                    yyterminate();
                }
            }

<MACRO_ARG_0>.  |
<MACRO_ARG_1>.  |
<MACRO_CALL>.   {
                yyerror(
                    "Macro should be followed by \"(\" but "
                    "receive %c instead at line no. %d\n",
                    *yytext, yylineno
                );
                abort();
}
.   {
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

char *str_replace(char *orig, char *rep, char *with) {
    char *result;
    char *ins;
    char *tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL;
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}

void expand_macro() {
    struct symbol *s;
    s = lookup(G_IDEN);

    char *a, *b;
    a = b = strdup(s -> val.s);

    for (int i = 0; i < 10; i++) {
        char r[3] = "$1";
        r[1] += i;

        a = str_replace(a, r, G_ARGS[i]);

        free(b);
        b = a;
    }

    yypush_buffer_state(YY_CURRENT_BUFFER);
    yy_scan_string(a);
    free(a);
}
