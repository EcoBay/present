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
lookup_0(char *sym) {
    struct symbol *s;
    unsigned h = hash(sym) & HASH_MOD;

    struct symTable *st = g_symtable;
    for (s = st -> table[h]; s; s = s -> next) {
        if (strcmp(s -> sym, sym) == 0) return s;
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
    unsigned h = hash(sym) & HASH_MOD;

    struct symbol *s0 = g_symtable -> table[h];
    if (!s0) return 1;

    struct symbol *s1 = s0 -> next;

    if (strcmp(s0 -> sym, sym) == 0) {
        g_symtable -> table[h] = s1;
        free(s0);
        return 0;
    }

    for (; s1; s0 = s1, s1 = s1 -> next) {
        if (strcmp(s1 -> sym, sym) == 0) {
            s0 -> next = s1 -> next;
            free(s1);
            return 0;
        }
    }

    return 1;
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
pushTable() {
    struct symTable *s;
    s = malloc(sizeof(struct symTable));

    s -> next = g_symtable;
    for (int i = 0; i < HASH_SIZE; i++) {
        s -> table[i] = NULL;
    }

    g_symtable = s;
}

struct symTable*
popTable() {
    struct symTable *s = g_symtable;
    g_symtable = g_symtable -> next;
    return s;
}

void
freeTable(struct symTable *tb) {
    for (int i = 0; i < HASH_SIZE; i++) {
        struct symbol *s = tb -> table[i];
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
    free(tb);
}
