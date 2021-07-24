%{
#include "present.h"
%}

%union {
    uint8_t t;
    char *s;
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

%type <t> primitive direction

%%
program: statement
       | program EOL statement
;

statement:
         | primitive_stmt
         | direction_stmt
;

primitive_stmt: primitive { draw($1, -1); }
              | primitive direction { draw($1, $2); }
;

direction_stmt: direction { changeDirection($1); }
;

text_list: TEXT
         | text_list TEXT
;

primitive: BOX          { $$ = 0; }
         | CIRCLE       { $$ = 1; }
         | ELLIPSE      { $$ = 2; }
         | ARC          { $$ = 3; }
         | LINE         { $$ = 4; }
         | ARROW        { $$ = 5; }
         | SPLINE       { $$ = 6; }
         | MOVE         { $$ = 7; }
         | text_list    { $$ = 8; }
;

direction: UP           { $$ = 0; }
         | DOWN         { $$ = 1; }
         | LEFT         { $$ = 2; }
         | RIGHT        { $$ = 3; }
;

%%
