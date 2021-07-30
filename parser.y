%{
#include "object.h"
#include "present.h"
#include <stddef.h>
%}

%union {
    char *s;
    struct primitive *p;
}

/* primitives */
%token BOX CIRCLE ELLIPSE ARC LINE ARROW SPLINE MOVE
%token <s> TEXT 

/* directions */
%token UP DOWN LEFT RIGHT

/* attributes */
%token HT WID RAD DIAM FROM TO AT WITH BY THEN DOTTED
%token CHOP LARROW RARROW LRARROW INVIS SOLID FILL SAME

%token EOL

%type <p> primitive

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
                $$ -> ht = 0.5;
                $$ -> wid = 0.75;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | CIRCLE       
            {
                $$ = newPrimitive(PRIM_CIRCLE);
                $$ -> rad = 0.25;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | ELLIPSE      
            {
                $$ = newPrimitive(PRIM_ELLIPSE);
                $$ -> ht = 0.5;
                $$ -> wid = 0.75;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | ARC          
            {
                $$ = newPrimitive(PRIM_ARC);
                $$ -> rad = 0.25;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | LINE         
            {
                $$ = newPrimitive(PRIM_LINE);
                $$ -> expr = 0.5;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | ARROW        
            {
                $$ = newPrimitive(PRIM_ARROW);
                $$ -> arrowStyle = 1;
                $$ -> expr = 0.5;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | SPLINE       
            {
                $$ = newPrimitive(PRIM_SPLINE);
                $$ -> expr = 0.5;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
         | MOVE         
            {
                $$ = newPrimitive(PRIM_MOVE);
                $$ -> expr = 0.5;

                $$ -> direction = getDirection();
                getCursor(&$$ -> start);
            }
        | primitive UP
            {
                if ($$ -> t > 2 && $$ -> t != 9) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> y += 0.5;
                    $$ -> flags |= 1;
                } // check if arc, line, arrow, spline, or move
                setDirection(0);
            }
        | primitive RIGHT
            {
                if ($$ -> t > 2 && $$ -> t != 9) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> x += 0.5;
                    $$ -> flags |= 1;
                } // check if arc, line, arrow, spline, or move
                setDirection(1);
            }
        | primitive DOWN
            {
                if ($$ -> t > 2 && $$ -> t != 9) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> y -= 0.5;
                    $$ -> flags |= 1;
                } // check if arc, line, arrow, spline, or move
                setDirection(2);
            }
        | primitive LEFT
            {
                if ($$ -> t > 2 && $$ -> t != 9) {
                    struct location *l;
                    l = getLastSegment($$);
                    l -> x -= 0.5;
                    $$ -> flags |= 1;
                } // check if arc, line, arrow, spline, or move
                setDirection(3);
            }
        | primitive THEN
            {
                struct location *l;
                if (! $$ -> segments){
                    l = getLastSegment($$);
                    float e = $$ -> expr;
                    switch ($$ -> direction) {
                        case 0: l -> y += e; break;
                        case 1: l -> x += e; break;
                        case 2: l -> y -= e; break;
                        case 3: l -> y -= e; break;
                    }
                }

                l = getLastSegment($$);
                l -> next = malloc(sizeof(struct location));
                l -> next -> next = NULL;
                l -> next -> x = l -> x;
                l -> next -> y = l -> y;
                $$ -> flags &= ~1;
            }
;

%%
