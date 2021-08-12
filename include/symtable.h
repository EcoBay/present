#ifndef SYM_TABLE_H
#define SYM_TABLE_H

#include "object.h"

#define HASH_SIZE 1024
#define HASH_MOD 1023

enum symType {
    SYM_DOUBLE,
    SYM_EVENT
};

union T {
    float d;
    struct event *e;
};

struct symbol {
    struct symbol *next;
    char *sym;
    enum symType t;
    union T val;
};

struct symTable {
    struct symTable *next;
    struct symbol *table[HASH_SIZE];
};

struct symbol* lookup(char*);
void defineSym(char *, enum symType, union T);
int resetSym(char *);

void pushTable();
struct symTable* popTable();

#define GET_FLOAT_SYM(Y, X) Y = lookup(X);                                      \
    if (!Y) {                                                                   \
        fprintf(stderr,"Error: \"" X "\" variable used before assignment\n");   \
        abort();                                                                \
    }                                                                           \
    if (Y -> t != SYM_DOUBLE) {                                                 \
        fprintf(stderr,"Error: \"" X "\" variable must be double\n");           \
        abort();                                                                \
    }

#endif
