%{
#include "present.h"
%}

%union {
    char s[32];
}

%token <s> PRIMITIVE DIRECTION
%token EOL

%%
program: statement
       | program EOL statement

statement:
         | primitive_stmt
         | direction_stmt
;

primitive_stmt: PRIMITIVE { draw($1, "same"); }
              | PRIMITIVE DIRECTION { draw($1, $2); }
;

direction_stmt: DIRECTION { changeDirection($1); }
;

%%
