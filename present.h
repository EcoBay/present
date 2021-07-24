#ifndef PRESENT_H
#define PRESENT_H

#include <stdint.h>

extern int yylineno;
int yylex(void);
void yyerror(char *s, ...);

void draw(uint8_t primitive, int direction);
void changeDirection(int direction);

#endif
