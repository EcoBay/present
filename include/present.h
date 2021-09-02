#ifndef PRESENT_H
#define PRESENT_H

#include <stdint.h>

extern int yylineno;
extern FILE* yyin;

int yylex(void);
void yyerror(char *s, ...);

char* str_replace(char*, char*, char*);

#endif
