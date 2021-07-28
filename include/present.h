#ifndef PRESENT_H
#define PRESENT_H

#include <stdint.h>

extern int yylineno;
int yylex(void);
void yyerror(char *s, ...);

#endif
