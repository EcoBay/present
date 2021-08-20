%{
#include "ast.h"
#include "present.h"
#include "symtable.h"
#include <math.h>
%}


%union {
    struct ast *a;
    char *s;
    struct primitive *p;
    int i;
    float d;
    struct color *c;
}

/* primitives */
%token BOX CIRCLE ELLIPSE ARC LINE ARROW SPLINE MOVE

/* keywords */
%token FOR

/* directions */
%token UP DOWN LEFT RIGHT

/* corners */
%token TOP BOT START END
%token DOT_N DOT_E DOT_W DOT_S DOT_C
%token DOT_NE DOT_NW DOT_SE DOT_SW DOT_START DOT_END

/* attributes */
%token HT WID RAD DIAM FROM TO AT WITH BY THEN DOTTED CW
%token DASHED CHOP LARROW RARROW LRARROW INVIS SOLID FILL SAME
u

/* text positioning */
%token CENTER LJUST RJUST ABOVE BELOW

/* build int functions */
%token SIN COS ATAN2 LOG EXP SQRT MAX MIN INT RAND ABS RGBA

/* values */
%token <s> TEXT HEXCOLOR
%token <d> NUMBER

/* present extensions */
%token KEYFRAME SCENE

/* easing function */
%token LINEAR SINE QUADRATIC CUBIC
%token IN OUT

/* time units */
%token SECONDS MILLISECONDS MINUTES

%token EOL

%type <a> program statement keyframe_stmt direction_stmt
%type <a> primitive duration expr color position
%type <i> positioning easing corner optional_corner

%left TEXT
%left LJUST RJUST ABOVE BELOW

%left LEFT RIGHT
%left CHOP SOLID DASHED DOTTED UP DOWN FILL

%left VARIABLE NUMBER '(' SIN COS ATAN2 LOG EXP SQRT MAX MIN INT RAND ABS
%left BOX CIRCLE ELLIPSE ARC LINE ARROW SPLINE '['

%left HT WID RAD DIAM FROM TO AT
%left ','

%left '+' '-'
%left '*' '/' '%'
%right '!'
%right '^'

%%
root: program   { eval($1); }

program: statement              { $$ = astStmt($1, NULL); }
       | program EOL statement  { $$ = astStmt($1, $3); }
       | program ';' statement  { $$ = astStmt($1, $3); }
;

statement: %empty               { $$ = NULL; }
         | primitive            { $$ = astDraw($1); }
         | direction_stmt       { $$ = $1; }
         | keyframe_stmt        { $$ = $1; }
;

keyframe_stmt: easing KEYFRAME              { $$ = astKF(NULL, $1); }
             | easing KEYFRAME FOR duration { $$ = astKF($4, $1); }
;

easing: %empty          { $$ = EASE_LINEAR; }
      | LINEAR          { $$ = EASE_LINEAR; }
      | SINE            { $$ = EASE_SINE; }
      | SINE IN         { $$ = EASE_IN_SINE; }
      | SINE OUT        { $$ = EASE_OUT_SINE; }
      | QUADRATIC       { $$ = EASE_QUAD; }
      | QUADRATIC IN    { $$ = EASE_IN_QUAD; }
      | QUADRATIC OUT   { $$ = EASE_OUT_QUAD; }
      | CUBIC           { $$ = EASE_CUBIC; }
      | CUBIC IN        { $$ = EASE_IN_CUBIC; }
      | CUBIC OUT       { $$ = EASE_OUT_CUBIC; }
;

duration: expr              { $$ = $1; }
        | expr SECONDS      { $$ = $1; }
        | expr MILLISECONDS { $$ = astOp('/', $1, astNum(1000.0)); }
        | expr MINUTES      { $$ = astOp('*', $1, astNum(60.0)); }
;

direction_stmt: UP      { $$ = astDir(0); }
              | RIGHT   { $$ = astDir(1); }
              | DOWN    { $$ = astDir(2); }
              | LEFT    { $$ = astDir(3); }
;

primitive: BOX
            { $$ = astPrim(PRIM_BOX); }
         | CIRCLE
            { $$ = astPrim(PRIM_CIRCLE); }
         | ELLIPSE      
            { $$ = astPrim(PRIM_ELLIPSE); }
         | ARC          
            { $$ = astPrim(PRIM_ARC); }
         | LINE         
            { $$ = astPrim(PRIM_LINE); }
         | ARROW        
            { $$ = astPrim(PRIM_ARROW); }
         | SPLINE       
            { $$ = astPrim(PRIM_SPLINE); }
         | MOVE         
            { $$ = astPrim(PRIM_MOVE); }
         | TEXT positioning
            {
                struct ast *t = astPrim(PRIM_TEXT_LIST);
                $$ = astAttr(t, ATTR_TXT, astTL(NULL, astText($1), $2));
            }
         | primitive UP
            { $$ = astAttr($1, ATTR_UP, NULL); }
         | primitive UP expr
            { $$ = astAttr($1, ATTR_UP, $3); }
         | primitive RIGHT
            { $$ = astAttr($1, ATTR_RIGHT, NULL); }
         | primitive RIGHT expr
            { $$ = astAttr($1, ATTR_RIGHT, $3); }
         | primitive DOWN
            { $$ = astAttr($1, ATTR_DOWN, NULL); }
         | primitive DOWN expr
            { $$ = astAttr($1, ATTR_DOWN, $3); }
         | primitive LEFT
            { $$ = astAttr($1, ATTR_LEFT, NULL); }
         | primitive LEFT expr
            { $$ = astAttr($1, ATTR_LEFT, $3); }
         | primitive BY expr ',' expr
            { $$ = astAttr($1, ATTR_BY, astOp(0, $3, $5)); }
         | primitive FROM position
            { $$ = astAttr($1, ATTR_FROM, $3); }
         | primitive TO position
            { $$ = astAttr($1, ATTR_TO, $3); }
         | primitive THEN
            { $$ = astAttr($1, ATTR_THEN, NULL); }
         | primitive CHOP
            { $$ = astAttr($1, ATTR_CHOP, astRef("circlerad")); }
         | primitive CHOP expr
            { $$ = astAttr($1, ATTR_CHOP, $3); }
         | primitive WITH optional_corner
            { $$ = astAttr($1, ATTR_WITH, astInt($3)); }
         | primitive WITH corner
            { $$ = astAttr($1, ATTR_WITH, astInt($3)); }
         | primitive AT position
            { $$ = astAttr($1, ATTR_AT, $3); }
         | primitive LARROW
            { $$ = astAttr($1, ATTR_LARROW, NULL); }
         | primitive RARROW
            { $$ = astAttr($1, ATTR_RARROW, NULL); }
         | primitive LRARROW
            { $$ = astAttr($1, ATTR_LRARROW, NULL); }
         | primitive CW
            { $$ = astAttr($1, ATTR_CW, NULL); }
         | primitive DASHED
            { $$ = astAttr($1, ATTR_DASHED, NULL); }
         | primitive DASHED expr
            { $$ = astAttr($1, ATTR_DASHED, $3); }
         | primitive DOTTED
            { $$ = astAttr($1, ATTR_DOTTED, NULL); }
         | primitive DOTTED expr
            { $$ = astAttr($1, ATTR_DOTTED, $3); }
         | primitive SOLID
            { $$ = astAttr($1, ATTR_SOLID, NULL); }
         | primitive INVIS
            { $$ = astAttr($1, ATTR_INVIS, NULL); }
         | primitive FILL
            {
                struct ast* c = astRGBA(
                    astNum(0), astNum(0), astNum(0),
                    astOp('*', astRef("fillval"), astNum(255)));
                $$ = astAttr($1, ATTR_FILL, c);
            }
         | primitive FILL color
            { $$ = astAttr($1, ATTR_FILL, $3); }
         | primitive TEXT positioning
            { $$ = astAttr($1, ATTR_TXT, astTL(NULL, astText($2), $3)); }
         | primitive HT expr
            { $$ = astAttr($1, ATTR_HT, $3); }
         | primitive WID expr
            { $$ = astAttr($1, ATTR_WID, $3); }
         | primitive RAD expr
            { $$ = astAttr($1, ATTR_RAD, $3); }
         | primitive DIAM expr
            { $$ = astAttr($1, ATTR_DIAM, $3); }
         | primitive expr                   %prec HT
            { $$ = astAttr($1, ATTR_EXPR, $2); }
;

positioning: %empty
            { $$ = 0; }
           | positioning CENTER
            { $$ = 0; }
           | positioning LJUST
            {
                $$ = $1;
                $$ &= ~3;
                $$ |= 1;
            }
           | positioning RJUST
            {
                $$ = $1;
                $$ &= ~3;
                $$ |= 2;
            }
           | positioning ABOVE
            {
                $$ = $1;
                $$ &= ~12;
                $$ |= 4;
            }
           | positioning BELOW
            {
                $$ = $1;
                $$ &= ~12;
                $$ |= 8;
            }
;

expr: NUMBER
        { $$ = astNum($1); }
    | expr '+' expr
        { $$ = astOp('+', $1, $3); }
    | expr '-' expr
        { $$ = astOp('-', $1, $3); }
    | expr '*' expr
        { $$ = astOp('*', $1, $3); }
    | expr '/' expr
        { $$ = astOp('/', $1, $3); }
    | expr '%' expr
        { $$ = astOp('%', $1, $3); }
    | expr '^' expr
        { $$ = astOp('^', $1, $3); }
    | '-' expr                              %prec '!'
        { $$ = astOp(AST_UNM, $2, NULL); }
    | '(' expr ')'
        { $$ = $2; }
    | SIN '(' expr ')'
        { $$ = astOp(AST_SIN, $3, NULL); }
    | COS '(' expr ')'
        { $$ = astOp(AST_COS, $3, NULL); }
    | ATAN2 '(' expr ',' expr ')'
        { $$ = astOp(AST_ATAN2, $3, $5); }
    | LOG '(' expr ')'
        { $$ = astOp(AST_LOG, $3, NULL); }
    | EXP '(' expr ')'
        { $$ = astOp(AST_EXP, $3, NULL); }
    | SQRT '(' expr ')'
        { $$ = astOp(AST_SQRT, $3, NULL); }
    | MAX '(' expr ',' expr ')'
        { $$ = astOp(AST_MAX, $3, $5); }
    | MIN '(' expr ',' expr ')'
        { $$ = astOp(AST_MIN, $3, $5); }
    | INT '(' expr ')'
        { $$ = astOp(AST_INT, $3, NULL); }
    | RAND '(' ')'
        { $$ = astOp(AST_RAND, NULL, NULL); }
    | ABS '(' expr ')'
        { $$ = astOp(AST_ABS , $3, NULL); }
    | '!' expr
        { $$ = astOp('!', $2, NULL); }
;

color: HEXCOLOR
        {
            char c[9];
            strncpy(c, $1, 9);

            int len = strlen($1);
            if (len < 6) {
                for (int i = 0; i < len; i++) {
                    c[i * 2] = $1[i];
                    c[i * 2 + 1] = $1[i];
                }
                len *= 2;
            }
            free($1);


            uint32_t i = strtoul(c, NULL, 16);
            struct ast *a;
            if (len < 8) {
                a = astOp('*', astRef("fillval"), astNum(255));
            } else {
                a = astNum((uint8_t) i);
            }
            struct ast *b = astNum((uint8_t) (i >> 8));
            struct ast *g = astNum((uint8_t) (i >> 16));
            struct ast *r = astNum((uint8_t) (i >> 24));
            $$ = astRGBA(r, g, b, a);
        }
     | RGBA '(' expr ',' expr ',' expr ',' expr ')'
        {
            struct ast *r = astOp('*', $3, astNum(255));
            struct ast *g = astOp('*', $5, astNum(255));
            struct ast *b = astOp('*', $7, astNum(255));
            struct ast *a = astOp('*', $9, astNum(255));
            $$ = astRGBA(r, g, b, a);
        }
;

position: expr ',' expr
            { $$ = astOp(0, $1, $3); }
        | '(' position ')'
            { $$ = $2; }
;

optional_corner: DOT_N      { $$ = 1; }
               | DOT_E      { $$ = 4; }
               | DOT_W      { $$ = 8; }
               | DOT_S      { $$ = 2; }
               | DOT_C      { $$ = 0; }
               | DOT_NE     { $$ = 5; }
               | DOT_NW     { $$ = 9; }
               | DOT_SE     { $$ = 6; }
               | DOT_SW     { $$ = 10; }
               | DOT_START  { $$ = 12; }
               | DOT_END    { $$ = 3; }
;

corner: TOP             { $$ = 1; }
      | BOT             { $$ = 2; }
      | LEFT            { $$ = 8; }
      | RIGHT           { $$ = 4; }
      | START           { $$ = 12; }
      | END             { $$ = 3; }
      | corner TOP
        {
            $$ = $1;
            $$ &= ~3;
            $$ |= 1;
        }
      | corner BOT
        {
            $$ = $1;
            $$ &= ~3;
            $$ |= 2;
        }
      | corner START    { $$ = 12; }
      | corner END      { $$ = 3; }
;
%%
