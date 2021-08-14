%{
#include "tex.h"
#include "object.h"
#include "present.h"
#include "symtable.h"
#include <stddef.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>

#define TEXTLIST(Y, S, P, L) struct symbol *s;              \
    GET_FLOAT_SYM(s, "ps");                                 \
    char *id = createTex(S, s -> val.d);                    \
    if (tex2SVG(id)) {                                      \
        yyerror("Error: Cannot create text \"%s\"\n", S);   \
        abort();                                            \
    }                                                       \
    RsvgHandle *h = getSVGHandler(id);;                     \
    Y = addTextList(h, P, L);                               \
    free(S);                                                \
    free(id)                                                
%}


%union {
    char *s;
    struct primitive *p;
    int i;
    float d;
    uint32_t c;
}

/* primitives */
%token BOX CIRCLE ELLIPSE ARC LINE ARROW SPLINE MOVE

/* directions */
%token UP DOWN LEFT RIGHT

/* attributes */
%token HT WID RAD DIAM FROM TO AT WITH BY THEN DOTTED CW
%token DASHED CHOP LARROW RARROW LRARROW INVIS SOLID FILL SAME

/* text positioning */
%token CENTER LJUST RJUST ABOVE BELOW

/* build int functions */
%token SIN COS ATAN2 LOG EXP SQRT MAX MIN INT RAND ABS

/* values */
%token <s> TEXT 
%token <d> NUMBER

%token EOL

%type <p> primitive
%type <i> positioning
%type <d> expr

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
program: statement
       | program EOL statement
       | program ';' statement
;

statement:
         | primitive        
            {
                preparePrimitive($1);
                newDrawEvent($1);
            }
         | direction_stmt  
;

direction_stmt: UP      { setDirection(0); }
              | RIGHT   { setDirection(1); }
              | DOWN    { setDirection(2); }
              | LEFT    { setDirection(3); }
;

primitive: BOX          
            {
                $$ = newPrimitive(PRIM_BOX);

                struct symbol *s;
                GET_FLOAT_SYM(s, "boxht");
                $$ -> ht = s -> val.d;
                GET_FLOAT_SYM(s, "boxwid");
                $$ -> wid = s -> val.d;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | CIRCLE       
            {
                $$ = newPrimitive(PRIM_CIRCLE);

                struct symbol *s;
                GET_FLOAT_SYM(s, "circlerad");
                $$ -> expr = s -> val.d;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | ELLIPSE      
            {
                $$ = newPrimitive(PRIM_ELLIPSE);

                struct symbol *s;
                GET_FLOAT_SYM(s, "ellipseht");
                $$ -> ht = s -> val.d;
                GET_FLOAT_SYM(s, "ellipsewid");
                $$ -> wid = s -> val.d;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | ARC          
            {
                $$ = newPrimitive(PRIM_ARC);

                struct symbol *s;
                GET_FLOAT_SYM(s, "arcrad");
                $$ -> expr = s -> val.d;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | LINE         
            {
                $$ = newPrimitive(PRIM_LINE);

                struct symbol *s;
                switch ($$ -> direction = getDirection()) {
                    case 0:
                    case 2:
                        GET_FLOAT_SYM(s, "lineht");
                    case 1:
                    case 3:
                        GET_FLOAT_SYM(s, "linewid");
                }

                $$ -> expr = s -> val.d;
                getCursor(&$$ -> start);
            }
         | ARROW        
            {
                $$ = newPrimitive(PRIM_ARROW);
                $$ -> arrowStyle = 1;

                struct symbol *s;
                switch ($$ -> direction = getDirection()) {
                    case 0:
                    case 2:
                        GET_FLOAT_SYM(s, "lineht");
                    case 1:
                    case 3:
                        GET_FLOAT_SYM(s, "linewid");
                }

                $$ -> expr = s -> val.d;
                getCursor(&$$ -> start);
            }
         | SPLINE       
            {
                $$ = newPrimitive(PRIM_SPLINE);

                struct symbol *s;
                switch ($$ -> direction = getDirection()) {
                    case 0:
                    case 2:
                        GET_FLOAT_SYM(s, "lineht");
                    case 1:
                    case 3:
                        GET_FLOAT_SYM(s, "linewid");
                }

                $$ -> expr = s -> val.d;
                getCursor(&$$ -> start);
            }
         | MOVE         
            {
                $$ = newPrimitive(PRIM_MOVE);

                struct symbol *s;
                switch ($$ -> direction = getDirection()) {
                    case 0:
                    case 2:
                        GET_FLOAT_SYM(s, "moveht");
                    case 1:
                    case 3:
                        GET_FLOAT_SYM(s, "movewid");
                }

                $$ -> expr = s -> val.d;
                getCursor(&$$ -> start);
            }
        | TEXT
            {
                $$ = newPrimitive(PRIM_TEXT_LIST);
                TEXTLIST($$ -> txt, $1, 0, NULL);

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
        | TEXT positioning
            {
                $$ = newPrimitive(PRIM_TEXT_LIST);
                TEXTLIST($$ -> txt, $1, $2, NULL);

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
        | primitive UP
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);

                    struct symbol *s;
                    if ($$ -> t == PRIM_MOVE) {
                        GET_FLOAT_SYM(s, "moveht");
                    } else {
                        GET_FLOAT_SYM(s, "lineht");
                    }
                    l -> y += s -> val.d;

                    $$ -> flags |= 1;
                    setDirection(0);
                } else if ($$ -> t == 3) {
                    $$ -> direction = 0;
                }
            }
        | primitive UP expr
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> y += $3;

                    $$ -> flags |= 1;
                    setDirection(0);
                } else if ($$ -> t == 3) {
                    $$ -> direction = 0;
                }
            }
        | primitive RIGHT
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);

                    struct symbol *s;
                    if ($$ -> t == PRIM_MOVE) {
                        GET_FLOAT_SYM(s, "movewid");
                    } else {
                        GET_FLOAT_SYM(s, "linewid");
                    }
                    l -> x += s -> val.d;

                    $$ -> flags |= 1;
                    setDirection(1);
                } else if ($$ -> t == 3) {
                    $$ -> direction = 1;
                }
            }
        | primitive RIGHT expr
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> x += $3;

                    $$ -> flags |= 1;
                    setDirection(1);
                } else if ($$ -> t == 3) {
                    $$ -> direction = 1;
                }
            }
        | primitive DOWN
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);

                    struct symbol *s;
                    if ($$ -> t == PRIM_MOVE) {
                        GET_FLOAT_SYM(s, "moveht");
                    } else {
                        GET_FLOAT_SYM(s, "lineht");
                    }
                    l -> y -= s -> val.d;

                    $$ -> flags |= 1;
                    setDirection(2);
                } else if ($$ -> t == 3) {
                    $$ -> direction = 2;
                }
            }
        | primitive DOWN expr
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> y -= $3;

                    $$ -> flags |= 1;
                    setDirection(2);
                } else if ($$ -> t == 3) {
                    $$ -> direction = 2;
                }
            }
        | primitive LEFT
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);

                    struct symbol *s;
                    if ($$ -> t == PRIM_MOVE) {
                        GET_FLOAT_SYM(s, "movewid");
                    } else {
                        GET_FLOAT_SYM(s, "linewid");
                    }
                    l -> x -= s -> val.d;

                    $$ -> flags |= 1;
                    setDirection(3);
                } else if ($$ -> t == 3) {
                    $$ -> direction = 3;
                }
            }
        | primitive LEFT expr
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> x -= $3;

                    $$ -> flags |= 1;
                    setDirection(3);
                } else if ($$ -> t == 3) {
                    $$ -> direction = 3;
                }
            }
        | primitive BY expr ',' expr
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> x += $3;
                    l -> y += $5;

                    $$ -> flags |= 1;
                }
            }
        | primitive THEN
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct location *l;
                    if (! $$ -> segments) {
                        l = getLastSegment($$);
                        float e = $$ -> expr;

                        struct symbol *s;
                        switch ($$ -> direction) {
                            case 0: l -> y += e; break;
                            case 1: l -> x += e; break;
                            case 2: l -> y -= e; break;
                            case 3: l -> x -= e; break;
                        }
                    } else {
                        l = getLastSegment($$);
                        l -> next = malloc(sizeof(struct location));
                        l -> next -> next = NULL;
                        l -> next -> x = l -> x;
                        l -> next -> y = l -> y;
                        $$ -> flags &= ~1;
                    }
                }
            }
        | primitive LARROW
            {
                $$ = $1;
                $$ -> arrowStyle &= ~3;
                $$ -> arrowStyle |=  2;
            }
        | primitive RARROW
            {
                $$ = $1;
                $$ -> arrowStyle &= ~3;
                $$ -> arrowStyle |=  1;
            }
        | primitive LRARROW
            {
                $$ = $1;
                $$ -> arrowStyle &= ~3;
                $$ -> arrowStyle |=  3;
            }
        | primitive CW
            {
                $$ = $1;
                $$ -> flags |= 2;
            }
        | primitive DASHED
            {
                $$ = $1;
                $$ -> spacing = 0.05;

                $$ -> flags &= ~12;
                $$ -> flags |= 4;
            }
        | primitive DASHED expr
            {
                $$ = $1;
                $$ -> spacing = $3;

                $$ -> flags &= ~12;
                $$ -> flags |= 4;
            }
        | primitive DOTTED
            {
                $$ = $1;
                $$ -> spacing = 0.05;

                $$ -> flags &= ~12;
                $$ -> flags |= 8;
            }
        | primitive DOTTED expr
            {
                $$ = $1;
                $$ -> spacing = $3;

                $$ -> flags &= ~12;
                $$ -> flags |= 8;
            }
        | primitive SOLID
            {
                $$ = $1;
                $$ -> flags &= ~16;
            }
        | primitive INVIS
            {
                $$ = $1;
                $$ -> flags |= 16;
            }
        | primitive FILL
            {
                $$ = $1;
                $$ -> flags |= 32;

                $$ -> fill = malloc(sizeof(struct color));
                $$ -> fill -> r = 0;
                $$ -> fill -> g = 0;
                $$ -> fill -> b = 0;

                struct symbol *s;
                GET_FLOAT_SYM(s, "fillval");
                $$ -> fill -> a = s -> val.d * 255;
            }
         | primitive TEXT
            {
                $$ = $1;
                TEXTLIST($$ -> txt, $2, 0, $$ -> txt);

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | primitive TEXT positioning
            {
                $$ = $1;
                TEXTLIST($$ -> txt, $2, $3, $$ -> txt);

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | primitive HT expr
            {
                $$ = $1;
                $$ -> ht = $3;
            }
         | primitive WID expr
            {
                $$ = $1;
                $$ -> wid = $3;
            }
         | primitive RAD expr
            {
                $$ = $1;
                $$ -> rad = $3;
                $$ -> flags |= 64;
            }
         | primitive DIAM expr
            {
                $$ = $1;
                $$ -> rad = $3 / 2.0;
                $$ -> flags |= 64;
            }
         | primitive expr               %prec HT
            {
                $$ = $1;
                $$ -> expr = $2; 
            }
;

positioning: CENTER { $$ = 0; }
           | LJUST  { $$ = 1; }
           | RJUST  { $$ = 2; }
           | ABOVE  { $$ = 3; }
           | BELOW  { $$ = 4; }
;

expr: NUMBER
    | expr '+' expr
        { $$ = $1 + $3; }
    | expr '-' expr
        { $$ = $1 - $3; }
    | expr '*' expr
        { $$ = $1 * $3; }
    | expr '/' expr
        { 
            if ($3 == 0.0) {
                yyerror("Error: division by zero\n");
                abort();
            }
            $$ = $1 / $3;
        }
    | expr '%' expr
        {
            if ($3 == 0.0) {
                yyerror("Error: modulos by zero\n");
                abort();
            }
            $$ = fmodf($1, $3);
        }
    | expr '^' expr
        {
            errno = 0;
            $$ = pow($1, $3);
            if (errno == EDOM) {
                yyerror("Error: arguments to '^' is out of domain\n");
                abort();
            }
        }
    | '-' expr                          %prec '!'
        { $$ = -$2; }
    | '(' expr ')'
        { $$ = $2; }
    | SIN '(' expr ')'
        {
            errno = 0;
            $$ = sin($3);
            if (errno == ERANGE) {
                yyerror("Error: sin result out of range\n");
                abort();
            }
        }
    | COS '(' expr ')'
        {
            errno = 0;
            $$ = cos($3);
            if (errno == ERANGE) {
                yyerror("Error: cos result out of range\n");
                abort();
            }
        }
    | ATAN2 '(' expr ',' expr ')'
        {
            errno = 0;
            $$ = atan2($3, $5);
            if (errno == EDOM) {
                yyerror("Error: atan2 argument out of domain\n");
                abort();
            }
            if (errno == ERANGE) {
                yyerror("Error: atan2 result out of range");
                abort();
            }
        }
    | LOG '(' expr ')'
        {
            errno = 0;
            $$ = log($3);
            if (errno == ERANGE) {
                yyerror("Error: log result out of range\n");
                abort();
            }
        }
    | EXP '(' expr ')'
        {
            errno = 0;
            $$ = pow(10.0, $3);
            if (errno == ERANGE) {
                yyerror("Error: exp result out of range\n");
                abort();
            }
        }
    | SQRT '(' expr ')'
        {
            errno = 0;
            $$ = sqrt($3);
            if (errno == EDOM) {
                yyerror("Error: sqrt result out of range\n");
                abort();
            }
        }
    | MAX '(' expr ',' expr ')'
        { $$ = $3 > $5 ? $3 : $5; }
    | MIN '(' expr ',' expr ')'
        { $$ = $3 < $5 ? $3 : $5; }
    | INT '(' expr ')'
        { $$ = $3 < 0 ? -floor(-$3) : floor($3); }
    | RAND '(' ')'
        { $$ = (float) rand() / (float) RAND_MAX; }
    | ABS '(' expr ')'
        { $$ = fabsf($3); }
    | '!' expr
        { $$ = ($2 == 0.0); }
;
%%
