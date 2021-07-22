#ifndef PRESENT_H
#define PRESENT_H

extern int yylineno;
int yylex(void);
void yyerror(char *s, ...);

void draw(char *primitive, char *direction);
void changeDirection(char *direction);

#endif
