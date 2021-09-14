#include "symtable.h"
#include <stdlib.h>

static struct symTable *g_symtable = NULL;

static unsigned
hash(char *s) {
    unsigned h = 0;
    while(*s++){
        h = h * 9 ^ *s;
    }
    return h;
}

struct symbol*
lookup(char *sym) {
    struct symbol *s;
    unsigned h = hash(sym) & HASH_MOD;

    for (struct symTable *st = g_symtable; st; st = st -> next) {
        for (s = st -> table[h]; s; s = s -> next) {
            if (strcmp(s -> sym, sym) == 0) return s;
        }
    }

    return NULL;
}

struct symbol*
removeSym(char *sym) {
    unsigned h = hash(sym) & HASH_MOD;
    struct symbol *s0 = g_symtable -> table[h];
    if (!s0) return NULL;

    struct symbol *s1 = s0 -> next;

    if (strcmp(s0 -> sym, sym) == 0) {
        g_symtable -> table[h] = s1;
        return s0;
    }

    for (; s1; s0 = s1, s1 = s1 -> next) {
        if (strcmp(s1 -> sym, sym) == 0) {
            s0 -> next = s1 -> next;
            return s1;
        }
    }

    return NULL;
}

void
setSym(char *sym, enum symType t, union T val) {
    unsigned h = hash(sym) & HASH_MOD;

    struct symbol *s = g_symtable -> table[h];

    while (s && strcmp(s -> sym, sym)) {
        s = s -> next;
    }

    if (s) {
        if (s -> t != t) {
            fprintf(stderr, "Error: Variable \"%s\" is already"
                    "defined under different type\n", sym);
            abort();
        }
        s -> val = val;
    } else {
        s = malloc(sizeof(struct symbol));
        s -> sym = strdup(sym);
        s -> next = g_symtable -> table[h];
        s -> t = t;
        s -> val = val;

        g_symtable -> table[h] = s;
    }
}

int
resetSym(char *sym) {
    struct symbol *s = removeSym(sym);
    if (s) {
        free(s);
        return 0;
    } else {
        return 1;
    }
}

void
clearSym() {
    for (int i = 0; i < HASH_SIZE; i++) {
        struct symbol *s = g_symtable -> table[i];
        while (s) {
            struct symbol *t = s;
            s = s -> next;
            if (t -> t == SYM_MACRO) {
                free(t -> val.s);
            }
            free(t -> sym);
            free(t);
        }
    }
}

void
addPrim(enum primitiveType T, struct event *e) {
    struct eventList *t = malloc(sizeof(struct eventList));
    t -> next = NULL;
    t -> e = e;

    struct eventList *l = getLast(g_symtable -> list[T]);
    if (l) {
        t -> prev = l;
        l -> next = t;
    } else {
        t -> prev = NULL;
        g_symtable -> list[T] = t;
    }
}

struct event*
getPrim(enum primitiveType T, int loc) {
    struct eventList *l = g_symtable -> list[T];

    for (int i = 1; i < loc; i++) { // One indexed
        if (!l) return NULL;
        l = l -> next;
    }

    return l ? l -> e : NULL;
}

struct event*
getPrim_r(enum primitiveType T, int loc) {
    struct eventList *l = getLast(g_symtable -> list[T]);

    for (int i = 1; i < loc; i++) { // One indexed
        if (!l) return NULL;
        l = l -> prev;
    }

    return l ? l -> e : NULL;
}

void
removePrim(struct primitive *p) {
    struct eventList *l0 = g_symtable -> list[p -> t];
    if (!l0) return;
    struct eventList *l1 = l0 -> next;

    if (p == l0 -> e -> pr) {
        g_symtable -> list[p -> t] = l1;
        free(l0);
    }


    for (; l1; l0 = l1, l1 = l1 -> next) {
        if (p == l1 -> e -> pr) {
            l0 -> next = l1 -> next;
            free(l1);
        }
    }
}

void
pushTable() {
    struct symTable *s;
    s = malloc(sizeof(struct symTable));

    s -> next = g_symtable;
    for (int i = 0; i < HASH_SIZE; i++) {
        s -> table[i] = NULL;
    }
    for (int i = 0; i < NUM_PRIM_TYPE; i++) {
        s -> list[i] = NULL;
    }

    g_symtable = s;
}

struct symTable*
switchTable(struct symTable *tb) {
    struct symTable *ret = g_symtable;
    g_symtable = tb;
    return ret;
}

struct symTable*
popTable() {
    if (!g_symtable) return NULL;
    struct symTable *s = g_symtable;
    g_symtable = g_symtable -> next;
    return s;
}

void
freeTable(struct symTable *tb) {
    for (int i = 0; i < HASH_SIZE; i++) {
        struct symbol *s = tb -> table[i];
        while (s) {
            struct symbol *next = s -> next;
            if (s -> t == SYM_MACRO) {
                free(s -> val.s);
            }
            free(s -> sym);
            free(s);
            s = next;
        }
    }
    for (int i =0; i < NUM_PRIM_TYPE; i++) {
        struct eventList *l = tb -> list[i];
        while (l) {
            struct eventList *next = l -> next;
            free(l);
            l = next;
        }
    }
    free(tb);
}

