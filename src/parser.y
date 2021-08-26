%{
#include "ast.h"
#include "present.h"
#include "symtable.h"
#include <math.h>

struct _text_list {
    struct _text_list *next;
    char *s;
};

struct _t {
    int  t;
    char *sym;
    enum symType varT;
    struct ast *a;
};

#define astRef_(a) astRef(strdup(a)) //astRef argument must not be a literal

%}

%union {
    struct ast *a;
    char *s;
    int i;
    float d;
    struct _text_list *tl;
}

/* primitives */
%token BOX CIRCLE ELLIPSE ARC LINE ARROW SPLINE MOVE

/* keywords */
%token FOR OF HERE AND BETWEEN DEFINE RESET PRINT

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
%token <s> TEXT HEXCOLOR IDENTIFIER LABEL TEMPLATE
%token <d> NUMBER

/* present extensions */
%token KEYFRAME SCENE

/* easing function */
%token LINEAR SINE QUADRATIC CUBIC
%token IN OUT

/* time units */
%token SECONDS MILLISECONDS MINUTES

%token EOL

%type <a> program statement element present label
%type <a> keyframe_stmt direction_stmt element_list
%type <a> primitive expr duration color position place
%type <a> position_not_place expr_pair reset prim_labels
%type <i> positioning easing corner optional_corner
%type <tl> iden_list

%left TEXT
%left LJUST RJUST ABOVE BELOW

%left LEFT RIGHT
%left CHOP SOLID DASHED DOTTED UP DOWN FILL

%left LABEL
%left IDENTIFIER NUMBER '(' SIN COS ATAN2 LOG EXP SQRT MAX MIN INT RAND ABS
%left HERE
%left BOX CIRCLE ELLIPSE ARC LINE ARROW SPLINE '['

%left HT WID RAD DIAM FROM TO AT
%left ','

%left BETWEEN OF
%left AND

%left '+' '-'
%left '*' '/' '%'
%right '!'
%right '^'

%%
root: program                       {
                                        eval($1);
                                        freeTree($1);
                                    }

program: statement                  { $$ = astStmt($1, NULL); }
       | program EOL statement      { $$ = astStmt($1, $3); }
       | program ';' statement      { $$ = astStmt($1, $3); }
;

statement: %empty                   { $$ = NULL; }
         | element                  { $$ = $1; }
         | present                  { $$ = $1; }
;

element: primitive
            { $$ = astDraw($1); }
       | prim_labels
       | IDENTIFIER '=' expr
            { $$ = astAsgn($1, SYM_DOUBLE, $3); }
       | direction_stmt
       | reset
       | PRINT expr
            { $$ = astPrn($2); }
       | '{' element_list '}'
            { $$ = astGrp($2); }
       | DEFINE IDENTIFIER TEMPLATE
            {
                $$ = NULL;
                union T v = {.s = $3};
                setSym($2, SYM_MACRO, v);
                free($2);
            }
       | DEFINE LABEL TEMPLATE
            {
                $$ = NULL;
                union T v = {.s = $3};
                setSym($2, SYM_MACRO, v);
                free($2);
            }
;

prim_labels: LABEL ':' primitive
                { $$ = astAsgn($1, SYM_EVENT, astDraw($3)); }
           | LABEL ':' EOL prim_labels
                { $$ = astAsgn($1, SYM_EVENT, $4); }
           | LABEL ':' prim_labels
                { $$ = astAsgn($1, SYM_EVENT, $3); }
;

element_list: %empty                    { $$ = NULL; }
            | element                   { $$ = astStmt($1, NULL); }
            | element_list EOL element  { $$ = astStmt($1, $3); }
            | element_list ';' element  { $$ = astStmt($1, $3); }
            | element_list EOL          { $$ = astStmt($1, NULL); }
            | element_list ';'          { $$ = astStmt($1, NULL); }
;

present: keyframe_stmt                      { $$ = $1; }
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
                $$ = astAttr(t, ATTR_TXT, astTL(astText($1), $2));
            }
         | '[' element_list ']'
            {
                struct ast *t = astPrim(PRIM_BLOCK);
                $$ = astAttr(t, ATTR_CH, $2);
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
            { $$ = astAttr($1, ATTR_CHOP, NULL); }
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
                    astOp('*', astRef_("fillval"), astNum(255)));
                $$ = astAttr($1, ATTR_FILL, c);
            }
         | primitive FILL color
            { $$ = astAttr($1, ATTR_FILL, $3); }
         | primitive TEXT positioning
            { $$ = astAttr($1, ATTR_TXT, astTL(astText($2), $3)); }
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
    | IDENTIFIER
        { $$ = astRef($1); }
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
                a = astOp('*', astRef_("fillval"), astNum(255));
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

position: position_not_place
        | place
        | '(' place ')'         { $$ = $2; }
;

position_not_place: expr_pair
                  | position '+' expr_pair
                    {
                        $$ = astOp(0,
                            astOp('+', $1 -> l, $3 -> l),
                            astOp('+', $1 -> r, $3 -> r)
                        );
                        free($1);
                        free($3);
                    }
                  | '(' position '+' expr_pair ')'
                    {
                        $$ = astOp(0,
                            astOp('+', $2 -> l, $4 -> l),
                            astOp('+', $2 -> r, $4 -> r)
                        );
                        free($2);
                        free($4);
                    }
                  | position '-' expr_pair
                    {
                        $$ = astOp(0,
                            astOp('-', $1 -> l, $3 -> l),
                            astOp('-', $1 -> r, $3 -> r)
                        );
                        free($1);
                        free($3);
                    }
                  | '(' position '-' expr_pair ')'
                    {
                        $$ = astOp(0,
                            astOp('-', $2 -> l, $4 -> l),
                            astOp('-', $2 -> r, $4 -> r)
                        );
                        free($2);
                        free($4);
                    }
                  | '(' position ',' position ')'
                    {
                        $$ = astOp(0, $2 -> l, $4 -> r);
                        free($2);
                        free($4);
                    }
                  | expr BETWEEN position AND position
                    {
                        $$ = astOp(0, NULL, NULL);
                        $$ -> l = astOp('+',
                            astOp('*',
                                astOp('-', astNum(1.0), $1),
                                $3 -> l),
                            astOp('*', $1, $5 -> l));
                        $$ -> r = astOp('+',
                            astOp('*',
                                astOp('-', astNum(1.0), $1),
                                $3 -> r),
                            astOp('*', $1, $5 -> r));

                        free($3);
                        free($5);
                    }
                  | '(' expr BETWEEN position AND position ')'
                    {
                        $$ = astOp(0, NULL, NULL);
                        $$ -> l = astOp('+',
                            astOp('*',
                                astOp('-', astNum(1.0), $2),
                                $4 -> l),
                            astOp('*', $2, $6 -> l));
                        $$ -> r = astOp('+',
                            astOp('*',
                                astOp('-', astNum(1.0), $2),
                                $4 -> r),
                            astOp('*', $2, $6 -> r));

                        free($4);
                        free($6);
                    }
                  | expr '<' position ',' position '>'
                    {
                        $$ = astOp(0, NULL, NULL);
                        $$ -> l = astOp('+',
                            astOp('*',
                                astOp('-', astNum(1.0), $1),
                                $3 -> l),
                            astOp('*', $1, $5 -> l));
                        $$ -> r = astOp('+',
                            astOp('*',
                                astOp('-', astNum(1.0), $1),
                                $3 -> r),
                            astOp('*', $1, $5 -> r));

                        free($3);
                        free($5);
                    }
                  | '(' expr '<' position ',' position '>' ')'
                    {
                        $$ = astOp(0, NULL, NULL);
                        $$ -> l = astOp('+',
                            astOp('*',
                                astOp('-', astNum(1.0), $2),
                                $4 -> l),
                            astOp('*', $2, $6 -> l));
                        $$ -> r = astOp('+',
                            astOp('*',
                                astOp('-', astNum(1.0), $2),
                                $4 -> r),
                            astOp('*', $2, $6 -> r));

                        free($4);
                        free($6);
                    }
;

reset: RESET            { $$ = astRst(NULL); }
     | RESET iden_list  { $$ = astRst($2); }
;

iden_list: IDENTIFIER
            {
                $$ = malloc(sizeof(struct _text_list));
                $$ -> next = NULL;
                $$ -> s = $1;
            }
         | LABEL
            {
                $$ = malloc(sizeof(struct _text_list));
                $$ -> next = NULL;
                $$ -> s = $1;
            }
         | iden_list IDENTIFIER
            {
                $$ = malloc(sizeof(struct _text_list));
                $$ -> next = $1;
                $$ -> s = $2;
            }
         | iden_list LABEL
            {
                $$ = malloc(sizeof(struct _text_list));
                $$ -> next = $1;
                $$ -> s = $2;
            }
;

expr_pair: expr ',' expr        { $$ = astOp(0, $1, $3); }
         | '(' expr_pair ')'    { $$ = $2; }
;

place: label optional_corner    { $$ = astLoc($1, $2); }
     | label                    { $$ = astLoc($1, 15); }
     | corner OF label          { $$ = astLoc($3, $1); }
     | optional_corner OF label { $$ = astLoc($3, $1); }
     | HERE                     { $$ = astHere(); }
;

label: LABEL            { $$ = astLbl($1); }
     | label '.' LABEL  { $$ = astTbl($1, astLbl($3)); }
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

corner: TOP  %prec TEXT { $$ = 1; }
      | BOT  %prec TEXT { $$ = 2; }
      | LEFT            { $$ = 8; }
      | RIGHT           { $$ = 4; }
      | START           { $$ = 12; }
      | END             { $$ = 3; }
      | TOP LEFT        { $$ = 9; }
      | TOP RIGHT       { $$ = 5; }
      | BOT LEFT        { $$ = 10; }
      | BOT RIGHT       { $$ = 6; }
      | LEFT TOP        { $$ = 9; }
      | RIGHT TOP       { $$ = 5; }
      | LEFT BOT        { $$ = 10; }
      | RIGHT BOT       { $$ = 6; }
      | CENTER          { $$ = 0; }
;
%%
