%{
#include "tex.h"
#include "object.h"
#include "present.h"
#include "symtable.h"
#include <stddef.h>

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
    struct textList *t;
    int i;
}

/* primitives */
%token BOX CIRCLE ELLIPSE ARC LINE ARROW SPLINE MOVE
%token <s> TEXT 

/* directions */
%token UP DOWN LEFT RIGHT

/* attributes */
%token HT WID RAD DIAM FROM TO AT WITH BY THEN DOTTED CW
%token CHOP LARROW RARROW LRARROW INVIS SOLID FILL SAME

/* text positioning */
%token CENTER LJUST RJUST ABOVE BELOW

%token EOL

%type <p> primitive
%type <t> text_list
%type <i> positioning

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
                $$ -> rad = s -> val.d;

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
                $$ -> rad = s -> val.d;

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
        | text_list
            {
                $$ = newPrimitive(PRIM_TEXT_LIST);
                $$ -> txt = $1;

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
                } // check if line, arrow, spline, or move
                else if ($$ -> t == 3) {
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
                } // check if line, arrow, spline, or move
                else if ($$ -> t == 3) {
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
                } // check if line, arrow, spline, or move
                else if ($$ -> t == 3) {
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
                } // check if line, arrow, spline, or move
                else if ($$ -> t == 3) {
                    $$ -> direction = 3;
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
;

positioning: CENTER { $$ = 0; }
           | LJUST  { $$ = 1; }
           | RJUST  { $$ = 2; }
           | ABOVE  { $$ = 3; }
           | BELOW  { $$ = 4; }
;

text_list: TEXT                         { TEXTLIST($$, $1, 0, NULL); }
         | TEXT positioning             { TEXTLIST($$, $1, $2, NULL); }
         | text_list TEXT               { TEXTLIST($$, $2, 0, $1); }
         | text_list TEXT positioning   { TEXTLIST($$, $2, $3, $1); }
;

%%
