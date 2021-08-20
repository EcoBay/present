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

#define FLOAT_TO_CHAR(Y, X) if (X < 0.0 || X > 1.0) {       \
        yyerror("Error: invalid rgba value of %.2f\n", X);  \
        abort();                                            \
    }                                                       \
    Y = X * 255

struct vec2d* initVec2d(float x, float y) {
    struct vec2d *a = malloc(sizeof(struct vec2d));
    a -> x = x;
    a -> y = y;
    return a;
}
%}


%union {
    char *s;
    struct primitive *p;
    struct vec2d *v;
    int i;
    float d;
    struct color *c;
}

/* primitives */
%token BOX CIRCLE ELLIPSE ARC LINE ARROW SPLINE MOVE

/* keywords */
%token FOR OF HERE AND BETWEEN

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
%token <s> TEXT HEXCOLOR IDENTIFIER LABEL
%token <d> NUMBER

/* present extensions */
%token KEYFRAME SCENE

/* easing function */
%token LINEAR SINE QUADRATIC CUBIC
%token IN OUT

/* time units */
%token SECONDS MILLISECONDS MINUTES

%token EOL

%type <p> primitive
%type <i> positioning easing corner
%type <i> optional_corner
%type <d> expr duration
%type <c> color
%type <v> position place position_not_place expr_pair

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
program: statement
       | program EOL statement
       | program ';' statement
;

statement: %empty
         | element
         | present
;

element: primitive        
            {
                preparePrimitive($1);
                newDrawEvent($1);
            }
       | IDENTIFIER ':' primitive
            {
                preparePrimitive($3);
                struct event *e = newDrawEvent($3);
                union T v = { .e = e };
                setSym($1, SYM_EVENT, v);
                free($1);
            }
       | IDENTIFIER '=' expr
            {
                union T v = { .d = $3 };
                setSym($1, SYM_DOUBLE, v);
                free($1);
            }
       | direction_stmt
;

present: keyframe_stmt
;

keyframe_stmt: easing KEYFRAME
                { newKeyframe(1.0, $1); }
             | easing KEYFRAME FOR duration
                { newKeyframe($4, $1); }
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
        | expr MILLISECONDS { $$ = $1 / 1000.0; }
        | expr MINUTES      { $$ = $1 * 60.0; }
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
        | primitive FROM position
            {
                $$ = $1;
                $$ -> start.x = $3 -> x;
                $$ -> start.y = $3 -> y;
            }
        | primitive TO position
            {
                $$ = $1;
                if ($$ -> t > 2 && $$ -> t < 8) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> x = $3 -> x;
                    l -> y = $3 -> y;

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
        | primitive CHOP
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    struct symbol *s;
                    GET_FLOAT_SYM(s, "circlerad");

                    if ($$ -> flags & 128) {
                        $$ -> chop2 = s -> val.d;
                    } else {
                        $$ -> chop1 = $$ -> chop2 = s -> val.d;
                        $$ -> flags |= 128;
                    }
                }
            }
        | primitive CHOP expr
            {
                $$ = $1;
                if ($$ -> t > 3 && $$ -> t < 8) {
                    if ($$ -> flags & 128) {
                        $$ -> chop2 = $3;
                    } else {
                        $$ -> chop1 = $$ -> chop2 = $3;
                        $$ -> flags |= 128;
                    }
                }
            }
        | primitive WITH optional_corner
            {
                $$ = $1;
                $$ -> with = $3;
            }
        | primitive WITH corner
            {
                $$ = $1;
                $$ -> with = $3;
            }
        | primitive AT position
            {
                $$ = $1;

                if (!$$ -> at) {
                    $$ -> at = malloc(sizeof(struct vec2d));
                }

                $$ -> at -> x = $3 -> x;
                $$ -> at -> y = $3 -> y;
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
        | primitive FILL color
            {
                $$ = $1;
                $$ -> flags |= 32;
                $$ -> fill = $3;
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
    | IDENTIFIER
        {
            struct symbol *s;
            GET_FLOAT_SYM(s, $1);
            $$ = s -> val.d;
            free($1);
        }
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

color: HEXCOLOR
        {
            $$ = malloc(sizeof(struct color));

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
            if (len < 8) {
                c[6] = 'F';
                c[7] = 'F';
            }

            uint32_t i = strtoul(c, NULL, 16);
            *(uint32_t*) $$ = i;
        }
     | RGBA '(' expr ',' expr ',' expr ',' expr ')'
        {
            $$ = malloc(sizeof(struct color));
            FLOAT_TO_CHAR($$ -> r, $3);
            FLOAT_TO_CHAR($$ -> g, $5);
            FLOAT_TO_CHAR($$ -> b, $7);
            FLOAT_TO_CHAR($$ -> a, $9);
        }
;

position: position_not_place
        | place
        | '(' place ')'         { $$ = $2; }
;

position_not_place: expr_pair
                  | position '+' expr_pair
                    {
                        $$ = initVec2d($1 -> x + $3 -> x,
                            $1 -> y + $3 -> y);
                        free($1);
                        free($3);
                    }
                  | '(' position '+' expr_pair ')'
                    {
                        $$ = initVec2d($2 -> x + $4 -> x,
                            $2 -> y + $4 -> y);
                        free($2);
                        free($4);
                    }
                  | position '-' expr_pair
                    {
                        $$ = initVec2d($1 -> x - $3 -> x,
                            $1 -> y - $3 -> y);
                        free($1);
                        free($3);
                    }
                  | '(' position '-' expr_pair ')'
                    {
                        $$ = initVec2d($2 -> x - $4 -> x,
                            $2 -> y - $4 -> y);
                        free($2);
                        free($4);
                    }
                  | '(' position ',' position ')'
                    {
                        $$ = initVec2d($2 -> x, $4 -> y);
                        free($2);
                        free($4);
                    }
                  | expr BETWEEN position AND position
                    {
                        $$ = initVec2d(0,0);
                        $$ -> x = (1 - $1)*$3 -> x + $1 * $5 -> x;
                        $$ -> y = (1 - $1)*$3 -> y + $1 * $5 -> y;
                        free($3);
                        free($5);
                    }
                  | '(' expr BETWEEN position AND position ')'
                    {
                        $$ = initVec2d(0,0);
                        $$ -> x = (1 - $2)*$4 -> x + $2 * $6 -> x;
                        $$ -> y = (1 - $2)*$4 -> y + $2 * $6 -> y;
                        free($4);
                        free($6);
                    }
                  | expr '[' position ',' position ']'
                    {
                        $$ = initVec2d(0,0);
                        $$ -> x = (1 - $1)*$3 -> x + $1 * $5 -> x;
                        $$ -> y = (1 - $1)*$3 -> y + $1 * $5 -> y;
                        free($3);
                        free($5);
                    }
                  | '(' expr '[' position ',' position ']' ')'
                    {
                        $$ = initVec2d(0,0);
                        $$ -> x = (1 - $2)*$4 -> x + $2 * $6 -> x;
                        $$ -> y = (1 - $2)*$4 -> y + $2 * $6 -> y;
                        free($4);
                        free($6);
                    }
;

expr_pair: expr ',' expr
            { $$ = initVec2d($1, $3); }
         | '(' expr_pair ')'
            { $$ = $2; }
;

place: LABEL optional_corner
        {
            struct symbol *s;
            GET_EVENT_SYM(s, $1);

            struct primitive *p = s -> val.e -> a.pr;
            struct vec2d *e = malloc(sizeof(struct vec2d));
            getLoc(p, e, $2);
            free($1);
            $$ = e;
        }
     | LABEL
        {
            struct symbol *s;
            GET_EVENT_SYM(s, $1);

            struct primitive *p = s -> val.e  -> a.pr;
            struct vec2d *e = malloc(sizeof(struct vec2d));
            getLoc(p, e, 0);
            free($1);
            $$ = e;
        }
     | corner OF LABEL
        {
            struct symbol *s;
            GET_EVENT_SYM(s, $3);

            struct primitive *p = s -> val.e  -> a.pr;
            struct vec2d *e = malloc(sizeof(struct vec2d));
            getLoc(p, e, $1);
            free($3);
            $$ = e;
        }
     | optional_corner OF LABEL
        {
            struct symbol *s;
            GET_EVENT_SYM(s, $3);

            struct primitive *p = s -> val.e  -> a.pr;
            struct vec2d *e = malloc(sizeof(struct vec2d));
            getLoc(p, e, $1);
            free($3);
            $$ = e;
        }
     | HERE
        {
            struct vec2d *e = malloc(sizeof(struct vec2d));
            getCursor(e);
            $$ = e;
        }
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
