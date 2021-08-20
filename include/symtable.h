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
void setSym(char *, enum symType, union T);
int resetSym(char *);

void pushTable();
struct symTable* popTable();

#define GET_FLOAT_SYM(Y, X) Y = lookup(X);                                      \
    if (!Y) {                                                                   \
        fprintf(stderr,"Error: \"%s\" variable used before assignment\n", X);   \
        abort();                                                                \
    }                                                                           \
    if (Y -> t != SYM_DOUBLE) {                                                 \
        fprintf(stderr,"Error: \"%s\" variable must be float\n", X);            \
        abort();                                                                \
    }

#define GET_EVENT_SYM(Y, X) Y = lookup(X);                                      \
    if (!Y) {                                                                   \
        fprintf(stderr,"Error: \"%s\" variable used before assignment\n", X);   \
        abort();                                                                \
    }                                                                           \
    if (Y -> t != SYM_EVENT) {                                                  \
        fprintf(stderr,"Error: \"%s\" variable must be event\n", X);            \
        abort();                                                                \
    }

#endif
