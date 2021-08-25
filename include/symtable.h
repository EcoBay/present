#ifndef SYM_TABLE_H
#define SYM_TABLE_H

#include "object.h"

#define HASH_SIZE 1024
#define HASH_MOD 1023

enum symType {
    SYM_DOUBLE,
    SYM_EVENT,
    SYM_MACRO,
};

union T {
    float d;
    struct event *e;
    char *s;
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
struct symbol* lookup_0(char*);
void setSym(char*, enum symType, union T);
int resetSym(char*);
void clearSym();

void pushTable();
struct symTable* popTable();
void freeTable(struct symTable*);

#endif
