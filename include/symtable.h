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

struct eventList {
    struct eventList *next;
    struct eventList *prev;
    struct event* e;
};

struct symTable {
    struct symTable *next;
    struct symbol *table[HASH_SIZE];
    struct eventList *list[NUM_PRIM_TYPE];
};

struct symbol* lookup(char*);
void setSym(char*, enum symType, union T);
int resetSym(char*);
void clearSym();

void addPrim(enum primitiveType, struct event*);
struct event* getPrim(enum primitiveType, int);
struct event* getPrim_r(enum primitiveType, int);

void pushTable();
struct symTable* switchTable(struct symTable *tb);
struct symTable* popTable();
void freeTable(struct symTable*);

#endif
