#ifndef AST_H
#define AST_H

#include "object.h"
#include "symtable.h"
#include <stdarg.h>

union ast_type {
    char *s;
    struct primitive *p;
    struct textList *tl;
    int i;
    float d;
    struct color *c;
    struct event *e;
    struct vec2d *v;
};

enum nodetype {
    AST_STMT = 256,
    AST_GRP,
    AST_FOR,
    AST_BY,
    AST_RPT,
    AST_IF,
    AST_PRIM,
    AST_ATTR,
    AST_TL,
    AST_RGBA, 
    AST_TEXT,
    AST_SPN,
    AST_INTL,
    AST_DRAW,
    AST_DIR,
    AST_KF,
    AST_SCN,
    AST_ASGN,
    AST_RST,
    AST_TBL,
    AST_LBL,
    AST_ORD,
    AST_PRN,

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

    /* condition */
    AST_EE,
    AST_NE,
    AST_LE,
    AST_GE,
    AST_OR,
    AST_AND,
    AST_XOR,
    AST_SAME,

    /* position */
    AST_LOC,
    AST_HERE,
    AST_VEC,
    AST_VADD,
    AST_VSUB,
    AST_VSEP,
    AST_VBET,
};

enum attrib{
    ATTR_TXT,
    ATTR_CH,
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
    ATTR_SAME,
    ATTR_EXPR,
    ATTR_FROM,
    ATTR_TO,
    ATTR_CHOP,
    ATTR_WITH,
    ATTR_AT,
    ATTR_ANIM,
};

struct ast {
    int t;
    struct ast *l;
    struct ast *r;
};

struct astList {
    struct ast **l;
    int nm;
    int maxn;
};

union ast_type eval(struct ast*, ...);
void freeTree(struct ast*);

struct ast* astStmt(struct ast*, struct ast*);
struct ast* astGrp(struct ast*);
struct ast* astFor(char *, struct ast*, struct ast*, struct ast*, struct ast*);
struct ast* astBy(struct ast*, int);
struct ast* astRpt(struct ast*, struct ast*);
struct ast* astIf(struct ast*, struct ast*, struct ast*);
struct ast* astPrim(enum primitiveType);
struct ast* astAttr(struct ast*, enum attrib, struct ast*);
struct ast* astTL(struct ast*, int);
struct ast* astRGBA(struct ast*, struct ast*, struct ast*, struct ast*);
struct ast* astText(char *s);
struct ast* astSpn(struct ast*, struct astList*);
struct ast* astDraw(struct ast*);
struct ast* astDir(int);
struct ast* astKF(struct ast*, int);
struct ast* astScn(struct ast*);
struct ast* astOp(int op, struct ast*, struct ast*);
struct ast* astNum(float d);
struct ast* astLoc(struct ast*, int);
struct ast* astHere();
struct ast* astVBet(struct ast*, struct ast*, struct ast*);
struct ast* astInt(int i);
struct ast* astRef(char*);
struct ast* astTbl(struct ast*, struct ast*);
struct ast* astLbl(char*);
struct ast* astOrd(int, enum primitiveType, int);
struct ast* astAsgn(char*, enum symType, struct ast*);
struct ast* astRst(void*);
struct ast* astPrn(struct ast*);

#endif
