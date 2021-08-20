#ifndef AST_H
#define AST_H

#include "object.h"

union ast_type {
    char *s;
    struct primitive *p;
    struct textList *tl;
    int i;
    float d;
    struct color *c;
};

enum nodetype {
    AST_STMT = 256,
    AST_PRIM,
    AST_ATTR,
    AST_TL,
    AST_RGBA, 
    AST_TEXT,
    AST_INTL,
    AST_DRAW,
    AST_DIR,
    AST_KF,

    /* expressions */
    AST_NUM,
    AST_REF,
    AST_UNM,
    AST_SIN,
    AST_COS,
    AST_ATAN2,
    AST_LOG,
    AST_EXP,
    AST_SQRT,
    AST_MAX,
    AST_MIN,
    AST_INT,
    AST_RAND,
    AST_ABS,
};

enum attrib{
    ATTR_TXT,
    ATTR_UP,
    ATTR_RIGHT,
    ATTR_DOWN,
    ATTR_LEFT,
    ATTR_BY,
    ATTR_THEN,
    ATTR_LARROW,
    ATTR_RARROW,
    ATTR_LRARROW,
    ATTR_CW,
    ATTR_DASHED,
    ATTR_DOTTED,
    ATTR_SOLID,
    ATTR_INVIS,
    ATTR_FILL,
    ATTR_HT,
    ATTR_WID,
    ATTR_RAD,
    ATTR_DIAM,
    ATTR_EXPR,
    ATTR_FROM,
    ATTR_TO,
    ATTR_CHOP,
    ATTR_WITH,
    ATTR_AT,
};

struct ast {
    int t;
    struct ast *l;
    struct ast *r;
};

union ast_type eval(struct ast*);
void freetree(struct ast*);

struct ast* astStmt(struct ast*, struct ast*);
struct ast* astPrim(enum primitiveType);
struct ast* astAttr(struct ast*, enum attrib, struct ast*);
struct ast* astTL(struct ast*, struct ast*, int);
struct ast* astRGBA(struct ast*, struct ast*, struct ast*, struct ast*);
struct ast* astText(char *s);
struct ast* astDraw(struct ast*);
struct ast* astDir(int);
struct ast* astKF(struct ast*, int);
struct ast* astOp(int op, struct ast *, struct ast *);
struct ast* astNum(float d);
struct ast* astInt(int i);
struct ast* astRef(char *s);

#endif