#include "ast.h"
#include "tex.h"
#include "present.h"
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#define INT(a) ((a) < 0 ? -floor(a) : floor(a))

struct _ast_prim {
    int t;
    enum primitiveType pt;
};

struct _ast_for {
    int t;
    char *var;
    struct ast *init;
    struct ast *term;
    struct ast *inc;
    struct ast *body;
};

struct _ast_if {
    int t;
    struct ast* cond;
    struct ast* succ;
    struct ast* fail;
};

struct _ast_attr {
    int t;
    enum attrib attr;
    struct ast* p;
    struct ast* val;
};

struct _ast_asgn {
    int t;
    char *sym;
    enum symType varT;
    struct ast *a;
};

struct _text_list {
    struct _text_list *next;
    char *s;
};

struct _ast_rst {
    int t;
    struct _text_list *tl;
}; 

struct _ast_tl {
    int t;
    struct ast* s;
    int p;
};

struct _ast_rgba {
    int t;
    struct ast* r;
    struct ast* g;
    struct ast* b;
    struct ast* a;
};

struct _ast_kf {
    int t;
    struct ast *duration;
    int easingFunc;
};

struct _ast_ord {
    int t;
    int i;
    enum primitiveType T;
    int r;
};

struct _ast_ai {
    int t;
    struct ast *a;
    int i;
};

struct _ast_vbet {
    int t;
    struct ast *p;
    struct ast *s;
    struct ast *e;
};

struct _ast_term {
    int t;
    union ast_type val;
};

struct ast*
astStmt(struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = AST_STMT;
    a -> l = l;
    a -> r = r;
    return a;
}

struct ast*
astGrp(struct ast *l) {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = AST_GRP;
    a -> l = l;
    a -> r = NULL;
    return a;
}

struct ast*
astFor(char *s, struct ast *init, struct ast *term,
struct ast *inc, struct ast *body) {
    struct _ast_for *a = malloc(sizeof(struct _ast_for));
    a -> t = AST_FOR;
    a -> var = s;
    a -> init = init;
    a -> term = term;
    a -> inc = inc;
    a -> body = body;

    return (struct ast*) a;
}

struct ast*
astBy(struct ast *i, int multiplicative) {
    struct _ast_ai *a = malloc(sizeof(struct _ast_ai));
    a -> t = AST_BY; 
    a -> a = i;
    a -> i = multiplicative;

    return (struct ast*) a;
}

struct ast*
astRpt(struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = AST_RPT;
    a -> l = l;
    a -> r = r;

    return a;
}

struct ast*
astIf(struct ast *cond, struct ast *succ, struct ast *fail) {
    struct _ast_if *a = malloc(sizeof(struct _ast_if));
    a -> t = AST_IF;
    a -> cond = cond;
    a -> succ = succ;
    a -> fail = fail;

    return (struct ast*) a;
}

struct ast*
astPrim(enum primitiveType pt) {
    struct _ast_prim *a = malloc(sizeof(struct _ast_prim));
    a -> t = AST_PRIM;
    a -> pt = pt;
    return (struct ast*) a;
}

struct ast*
astAttr(struct ast *p, enum attrib attr, struct ast *val) {
    struct _ast_attr *a = malloc(sizeof(struct _ast_attr));
    a -> t = AST_ATTR;
    a -> attr = attr;
    a -> p = p;
    a -> val = val;
    return (struct ast*) a;
}

struct ast*
astTL(struct ast *s, int p) {
    struct _ast_tl *a = malloc(sizeof(struct _ast_tl));
    a -> t = AST_TL;
    a -> s = s;
    a -> p = p;
    return (struct ast*) a;
}

struct ast*
astRGBA(struct ast *r, struct ast *g, struct ast *b, struct ast *ap) {
    struct _ast_rgba *a = malloc(sizeof(struct _ast_rgba));
    a -> t = AST_RGBA;
    a -> r = r;
    a -> g = g;
    a -> b = b;
    a -> a = ap;
    return (struct ast*) a;
}

struct ast*
astText(char *s){
    struct _ast_term *a = malloc(sizeof(struct _ast_term));
    a -> t = AST_TEXT;
    a -> val.s = s;
    return (struct ast*) a;
}

struct ast*
astDraw(struct ast *l) {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = AST_DRAW;
    a -> l = l;
    a -> r = NULL;
    return a;
}

struct ast*
astDir(int i) {
    struct _ast_term *a = malloc(sizeof(struct _ast_term));
    a -> t = AST_DIR; 
    a -> val.i = i;
    return (struct ast*) a;
}

struct ast*
astKF(struct ast *duration, int easingFunc) {
    struct _ast_kf *a = malloc(sizeof(struct _ast_kf));
    a -> t = AST_KF;
    a -> duration = duration;
    a -> easingFunc = easingFunc;
    return (struct ast*) a;
}

struct ast*
astOp(int op, struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = op;
    a -> l = l;
    a -> r = r;
    return a;
}

struct ast*
astNum(float d) {
    struct _ast_term *a = malloc(sizeof(struct _ast_term));
    a -> t = AST_NUM;
    a -> val.d = d;
    return (struct ast*) a;
}

struct ast*
astLoc(struct ast* e, int corner) {
    struct _ast_ai *a = malloc(sizeof(struct _ast_ai));
    a -> t = AST_LOC;
    a -> a = e;
    a -> i = corner;

    return (struct ast*) a;
}

struct ast*
astHere() {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = AST_HERE;
    a -> l = NULL;
    a -> r = NULL;

    return a;
}

struct ast*
astVBet(struct ast *p, struct ast *s, struct ast *e) {
    struct _ast_vbet *a = malloc(sizeof(struct _ast_vbet));
    a -> t = AST_VBET;
    a -> p = p;
    a -> s = s;
    a -> e = e;

    return (struct ast*) a;
}

struct ast*
astInt(int i) {
    struct _ast_term *a = malloc(sizeof(struct _ast_term));
    a -> t = AST_INTL;
    a -> val.i = i;
    return (struct ast*) a;
}

struct ast*
astRef(char *s) {
    struct _ast_term *a = malloc(sizeof(struct _ast_term));
    a -> t = AST_REF; 
    a -> val.s = s;
    return (struct ast*) a;
}

struct ast*
astTbl(struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = AST_TBL;
    a -> l = l;
    a -> r = r;

    return a;
}

struct ast*
astLbl(char *s) {
    struct _ast_term *a = malloc(sizeof(struct _ast_term));
    a -> t = AST_LBL; 
    a -> val.s = s;
    return (struct ast*) a;
}

struct ast*
astOrd(int i, enum primitiveType T, int r) {
    struct _ast_ord *a = malloc(sizeof(struct _ast_ord));
    a -> t = AST_ORD;
    a -> i = i;
    a -> T = T;
    a -> r = r;
    return (struct ast*) a;
}

struct ast*
astAsgn(char *sym, enum symType T, struct ast* val) {
    struct _ast_asgn *a = malloc(sizeof(struct _ast_asgn));
    a -> t = AST_ASGN;
    a -> sym = sym;
    a -> varT = T;
    a -> a = val;
    return (struct ast*) a;
}

struct ast*
astRst(void *tl) {
    struct _ast_rst *a = malloc(sizeof(struct _ast_rst));
    a -> t = AST_RST;
    a -> tl = tl;
    return (struct ast*) a;
}

struct ast*
astPrn(struct ast* l){
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = AST_PRN;
    a -> l = l;
    a -> r = NULL;
    return a;
}

static struct primitive*
evalPrim(struct _ast_prim *a) {
    struct primitive *p = newPrimitive(a -> pt);
    struct symbol *s;
    uint8_t direction = getDirection();

    switch (a -> pt) {
        case PRIM_BOX:
            {
                s = lookup("boxht");
                p -> ht = s -> val.d;
                s = lookup("boxwid");
                p -> wid = s -> val.d;
            }
            break;
        case PRIM_CIRCLE:
            {
                s = lookup("circlerad");
                p -> expr = s -> val.d;
            }
            break;
        case PRIM_ELLIPSE:
            {
                s = lookup("ellipseht");
                p -> ht = s -> val.d;
                s = lookup("ellipsewid");
                p -> wid = s -> val.d;
            }
            break;
        case PRIM_ARC:
            {
                s = lookup("arcrad");
                p -> expr = s -> val.d;
            }
            break;
        case PRIM_ARROW:
            {
                p -> arrowStyle = 1;
            }
        case PRIM_LINE:
        case PRIM_SPLINE:
            switch (direction) {
                case 0:
                case 2:
                    s = lookup("lineht");
                    break;
                case 1:
                case 3:
                    s = lookup("linewid");
                    break;
            }
            p -> expr = s -> val.d;
            break;
        case PRIM_MOVE:
            switch (direction) {
                case 0:
                case 2:
                    s = lookup("moveht");
                    break;
                case 1:
                case 3:
                    s = lookup("movewid");
                    break;
            }
            p -> expr = s -> val.d;
            break;
        case PRIM_BLOCK: break;
        case PRIM_TEXT_LIST: break;
    }

    p -> direction = direction;
    getCursor(&p -> start);
    return p;
}

static struct primitive*
evalAttr(struct _ast_attr *a) {
    struct primitive *p = eval(a -> p).p;
    struct symbol *s;

    switch (a -> attr) {
        case ATTR_TXT:
            {
                p -> txt = eval(a -> val, p -> txt).tl;
            }
            break;
        case ATTR_CH:
            {
                pushTable();
                struct primitive *t = g_parent;
                g_parent = p;
                eval(a -> val);
                p -> tb = popTable();
                g_parent = t;
            }
            break;

        case ATTR_UP:
            if (p -> t > 3 && p -> t < 8) {
                struct location *l;
                l = getLastSegment(p);

                if (a -> val) {
                    l -> y += eval(a -> val).d;
                } else {
                    if (p -> t == PRIM_MOVE) {
                        s = lookup("moveht");
                    } else {
                        s = lookup("lineht");
                    }
                    l -> y += s -> val.d;
                }

                p -> flags |= 1;
                setDirection(0);
            } else if (p -> t == 3) {
                p -> direction = 0;
            }
            break;
        case ATTR_RIGHT:
            if (p -> t > 3 && p -> t < 8) {
                struct location *l;
                l = getLastSegment(p);

                if (a -> val) {
                    l -> x += eval(a -> val).d;
                } else {
                    if (p -> t == PRIM_MOVE) {
                        s = lookup("movewid");
                    } else {
                        s = lookup("linewid");
                    }
                    l -> x += s -> val.d;
                }

                p -> flags |= 1;
                setDirection(1);
            } else if (p -> t == 3) {
                p -> direction = 1;
            }
            break;
        case ATTR_DOWN:
            if (p -> t > 3 && p -> t < 8) {
                struct location *l;
                l = getLastSegment(p);

                if (a -> val) {
                    l -> y -= eval(a -> val).d;
                } else {
                    if (p -> t == PRIM_MOVE) {
                        s = lookup("moveht");
                    } else {
                        s = lookup("lineht");
                    }
                    l -> y -= s -> val.d;
                }

                p -> flags |= 1;
                setDirection(2);
            } else if (p -> t == 3) {
                p -> direction = 2;
            }
            break;
        case ATTR_LEFT:
            if (p -> t > 3 && p -> t < 8) {
                struct location *l;
                l = getLastSegment(p);

                if (a -> val) {
                    l -> x -= eval(a -> val).d;
                } else {
                    if (p -> t == PRIM_MOVE) {
                        s = lookup("movewid");
                    } else {
                        s = lookup("linewid");
                    }
                    l -> x -= s -> val.d;
                }

                p -> flags |= 1;
                setDirection(3);
            } else if (p -> t == 3) {
                p -> direction = 3;
            }
            break;
        case ATTR_BY:
            if (p -> t > 3 && p -> t < 8) {
                struct location *l;
                struct vec2d *v = eval(a -> val).v;

                l = getLastSegment(p);
                l -> x += v -> x;
                l -> y += v -> y;
                free(v);

                p -> flags |= 1;
            }
            break;
        case ATTR_THEN:
            if (p -> t > 3 && p -> t < 8) {
                struct location *l;
                if (!p -> segments) {
                    l = getLastSegment(p);
                    float e = p -> expr;

                    switch (p -> direction) {
                        case 0: l -> y += e; break;
                        case 1: l -> x += e; break;
                        case 2: l -> y -= e; break;
                        case 3: l -> x -= e; break;
                    }
                } else {
                    l = getLastSegment(p);
                    l -> next = malloc(sizeof(struct location));
                    l -> next -> next = NULL;
                    l -> next -> x = l -> x;
                    l -> next -> y = l -> y;
                    p -> flags &= ~1;
                }
            }
            break;
        case ATTR_LARROW:
            {
                p -> arrowStyle &= ~3;
                p -> arrowStyle |= 2;
            }
            break;
        case ATTR_RARROW:
            {
                p -> arrowStyle &= ~3;
                p -> arrowStyle |= 1;
            }
            break;
        case ATTR_LRARROW:
            {
                p -> arrowStyle |= 3;
            }
            break;
        case ATTR_CW:
            {
                p -> flags |= 2;
            }
            break;
        case ATTR_DASHED:
            {
                p -> flags &= ~12;
                p -> flags |= 4;

                p -> spacing = a -> val ? eval(a -> val).d :
                    lookup("dashwid") -> val.d;
            }
            break;
        case ATTR_DOTTED:
            {
                p -> flags &= ~12;
                p -> flags |= 8;

                p -> spacing  = a -> val ? eval(a -> val).d :
                    lookup("dashwid") -> val.d;
            }
            break;
        case ATTR_SOLID:
            {
                p -> flags &= ~16;
            }
            break;
        case ATTR_INVIS:
            {
                p -> flags |= 16;
            }
            break;
        case ATTR_FILL:
            {
                p -> flags |= 32;
                p -> fill = eval(a -> val).c;
            }
            break;
        case ATTR_HT:
            {
                p -> ht = eval(a -> val).d;
            }
            break;
        case ATTR_WID:
            {
                p -> wid = eval(a -> val).d;
            }
            break;
        case ATTR_RAD:
            {
                p -> rad = eval(a -> val).d;
                p -> flags |= 64;
            }
            break;
        case ATTR_DIAM:
            {
                p -> rad = eval(a -> val).d / 2.0;
                p -> flags |= 64;
            }
            break;
        case ATTR_EXPR:
            {
                p -> expr = eval(a -> val).d / 2.0;
            }
            break;
        case ATTR_FROM:
            {
                struct vec2d *v = eval(a -> val).v;
                p -> start.x = v -> x;
                p -> start.y = v -> y;
                free(v);
            }
            break;
        case ATTR_TO:
            if (p -> t > 2 && p -> t < 8) {
                struct location *l;
                l = getLastSegment(p);
                struct vec2d *v = eval(a -> val).v;

                l -> x = v -> x;
                l -> y = v -> y;
                free(v);

                p -> flags |= 1;
            }
            break;
        case ATTR_CHOP:
            if (p -> t > 2 && p -> t < 8) {
                float cd = a -> val ? eval(a -> val).d
                    : lookup("circlerad") -> val.d;
                if (p -> flags & 128) {
                    p -> chop2 = cd;
                } else {
                    p -> chop1 = p -> chop2 = cd;
                    p -> flags |= 128;
                }
            }
            break;
        case ATTR_WITH:
            {
                p -> with = eval(a -> val).i;
            }
            break;
        case ATTR_AT:
            {
                if (!p -> at) {
                    p -> at = malloc(sizeof(struct vec2d));
                }

                struct vec2d *v = eval(a -> val).v;
                p -> at -> x = v -> x;
                p -> at -> y = v -> y;
                free(v);
            }
            break;
    }

    return p;
}

union ast_type
eval(struct ast *a, ...) {
    union ast_type ret = {.i = 0};
    if (!a) return ret;
    struct symbol *s;

    va_list args;
    va_start(args, a);

    switch (a -> t) {
        /* expresions */
        case AST_NUM:
            {
                ret.d = ((struct _ast_term*) a) -> val.d;
            }
            break;
        case AST_REF:
            {
                s = lookup(((struct _ast_term*) a) -> val.s);
                ret.d = s -> val.d;
            }
            break;
        case '+':
            {
                ret.d = eval(a -> l).d + eval(a -> r).d;
            }
            break;
        case '-':
            {
                ret.d = eval(a -> l).d + eval(a -> r).d;
            }
            break;
        case '*':
            {
                ret.d = eval(a -> l).d * eval(a -> r).d;
            }
            break;
        case '/':
            {
                float divisor = eval(a -> r).d;
                if (divisor == 0.0) {
                    yyerror("Error: division by zero\n");
                    abort();
                }
                ret.d = eval(a -> l).d / divisor;
            }
            break;
        case '%':
            {
                float modulo = eval(a -> r).d;
                if (modulo == 0.0) {
                    yyerror("Error: modulos by zero\n");
                    abort();
                }
                ret.d = fmodf(eval(a -> l).d, modulo);
            }
            break;
        case '^':
            {
                errno = 0;
                ret.d = pow(eval(a -> l).d, eval(a -> r).d);
                if (errno == EDOM) {
                    yyerror("Error: arguments to '&' is out of domain\n");
                    abort();
                }
            }
            break;
        case AST_UNM:
            {
                ret.d = - eval(a -> l).d;
            }
            break;
        case AST_SIN:
            {
                errno = 0;
                ret.d = sin(eval(a -> l).d);
                if (errno == ERANGE) {
                    yyerror("Error: sin result out of range\n");
                    abort();
                }
            }
            break;
        case AST_COS:
            {
                errno = 0;
                ret.d = cos(eval(a -> l).d);
                if (errno == ERANGE) {
                    yyerror("Error: cos result out of range\n");
                    abort();
                }
            }
            break;
        case AST_ATAN2:
            {
                errno = 0;
                ret.d = atan2(eval(a -> l).d, eval(a -> r).d);
                if (errno == EDOM) {
                    yyerror("Error: atan2 argument out of domain\n");
                    abort();
                }
                if (errno == ERANGE) {
                    yyerror("Error: atan2 result out of range\n");
                    abort();
                }
            }
            break;
        case AST_LOG:
            {
                errno = 0;
                ret.d = log(eval(a -> l).d);
                if (errno == ERANGE) {
                    yyerror("Error: log result out of range\n");
                    abort();
                }
            }
            break;
        case AST_EXP:
            {
                errno = 0;
                ret.d = pow(10.0, eval(a -> l).d);
                if (errno == ERANGE) {
                    yyerror("Error: exp result out of range\n");
                    abort();
                }
            }
            break;
        case AST_SQRT:
            {
                errno = 0;
                ret.d = sqrt(eval(a -> l).d);
                if (errno == EDOM) {
                    yyerror("error: sqrt result out of range\n");
                    abort();
                }
            }
            break;
        case AST_MAX:
            {
                ret.d = MAX(eval(a -> l).d, eval(a -> r).d);
            }
            break;
        case AST_MIN:
            {
                ret.d = MIN(eval(a -> l).d, eval(a -> r).d);
            }
            break;
        case AST_INT:
            {
                ret.d = INT(eval(a -> l).d);
            }
            break;
        case AST_RAND:
            {
                ret.d = (float) rand() / (float) RAND_MAX;
            }
            break;
        case AST_ABS:
            {
                ret.d = fabsf(eval(a -> l).d);
            }
            break;

        /* condition */
        case '!':
            {
                ret.i = (eval(a -> l).d == 0.0);
            }
            break;
        case '<':
            {
                ret.i = eval(a -> l).d < eval(a -> r).d;
            }
            break;
        case '>':
            {
                ret.i = eval(a -> l).d > eval(a -> r).d;
            }
            break;
        case AST_EE:
            {
                ret.i = eval(a -> l).d == eval(a -> r).d;
            }
            break;
        case AST_NE:
            {
                ret.i = eval(a -> l).d != eval(a -> r).d;
            }
            break;
        case AST_LE:
            {
                ret.i = eval(a -> l).d <= eval(a -> r).d;
            }
            break;
        case AST_GE:
            {
                ret.i = eval(a -> l).d >= eval(a -> r).d;
            }
            break;
        case AST_OR:
            {
                ret.i = eval(a -> l).i || eval(a -> r).i;
            }
            break;
        case AST_AND:
            {
                ret.i = eval(a -> l).i && eval(a -> r).i;
            }
            break;
        case AST_XOR:
            {
                ret.i = !eval(a -> l).i != !eval(a -> r).i;
            }
            break;
        case AST_SAME:
            {
                ret.i = !eval(a -> l).i == !eval(a -> r).i;
            }
            break;

        /* position */
        case AST_LOC:
            {
                struct _ast_ai *t = (struct _ast_ai*) a;
                ret.v = malloc(sizeof(struct vec2d));

                struct primitive *p;
                p = eval(t -> a).e -> a.pr;
                getLoc(p, ret.v, t -> i);
            }
            break;
        case AST_HERE:
            {
                ret.v = malloc(sizeof(struct vec2d));
                getCursor(ret.v);
            }
            break;
        case AST_VEC:
            {
                ret.v = malloc(sizeof(struct vec2d));
                ret.v -> x = eval(a -> l).d;
                ret.v -> y = eval(a -> r).d;
            }
            break;
        case AST_VADD:
            {
                ret.v = malloc(sizeof(struct vec2d));
                struct vec2d *l, *r;
                l = eval(a -> l).v;
                r = eval(a -> r).v;
                
                ret.v -> x = l -> x + r -> x;
                ret.v -> y = l -> y + r -> y;
                free(l);
                free(r);
            }
            break;
        case AST_VSUB:
            {
                ret.v = malloc(sizeof(struct vec2d));
                struct vec2d *l, *r;
                l = eval(a -> l).v;
                r = eval(a -> r).v;
                
                ret.v -> x = l -> x - r -> x;
                ret.v -> y = l -> y - r -> y;
                free(l);
                free(r);
            }
            break;
        case AST_VSEP:
            {
                ret.v = malloc(sizeof(struct vec2d));
                struct vec2d *l, *r;
                l = eval(a -> l).v;
                r = eval(a -> r).v;
                
                ret.v -> x = l -> x;
                ret.v -> y = r -> y;
                free(l);
                free(r);
            }
            break;
        case AST_VBET:
            {
                struct _ast_vbet* t = (struct _ast_vbet*) a;
                float p = eval(t -> p).d;
                struct vec2d *s, *e;
                s = eval(t -> s).v;
                e = eval(t -> e).v;

                ret.v = malloc(sizeof(struct vec2d));
                ret.v -> x = (1.0 - p) * s -> x + p * e -> x;
                ret.v -> y = (1.0 - p) * s -> y + p * e -> y;

                free(s);
                free(e);
            }
            break;

        case AST_STMT:
            {
                eval(a -> l);
                eval(a -> r);
            }
            break;
        case AST_GRP:
            {
                struct vec2d c;
                getCursor(&c);
                int dir = getDirection();
                pushTable();

                eval(a -> l);

                freeTable(popTable());
                setDirection(dir);
                setCursor(&c);
            }
            break;
        case AST_FOR:
            {
                struct _ast_for *t = (struct _ast_for*) a;
                struct symbol *s = lookup(t -> var);
                union T prev_val;

                if (s) {
                    prev_val.d = s -> val.d;
                }

                float i = eval(t -> init).d;
                float term = eval(t -> term).d;

                while (i <= term) {
                    setSym(t -> var, SYM_DOUBLE, (union T) {.d = i});
                    eval(t -> body);
                    i = eval(t -> inc, (double) i).d;
                }

                if (s) {
                    setSym(t -> var, SYM_DOUBLE, prev_val);
                }
            }
            break;
        case AST_BY:
            {
                struct _ast_ai *t = (struct _ast_ai*) a;
                if (t -> i) {
                    float f = va_arg(args, double);
                    if (f) {
                        ret.d = f * eval(t -> a).d;
                    } else {
                        fprintf(stderr, "Error: multiplicative loop initial "
                                "value is 0 so causing it to not terminate\n");
                        abort();
                    }
                } else {
                    ret.d = va_arg(args, double) + eval(t -> a).d;
                }
            }
            break;
        case AST_RPT:
            for (int i = 0; i < eval(a -> l).d; i++) {
                eval(a -> r);
            }
            break;
        case AST_IF:
            {
                struct _ast_if *t = (struct _ast_if*) a;
                if (eval(t -> cond).i) {
                    eval(t -> succ);
                } else {
                    eval(t -> fail);
                }
            }
            break;
        case AST_PRIM:
            {
                ret.p = evalPrim( (struct _ast_prim*) a );
            }
            break;
        case AST_ATTR:
            {
                ret.p = evalAttr( (struct _ast_attr*) a );
            }
            break;
        case AST_TL:
            {
                s = lookup("ps");
                struct _ast_tl *t = (struct _ast_tl*) a;
                char *str = eval(t -> s).s;
                char *id = createTex(str, s -> val.d);
                if (tex2SVG(id)) {
                    yyerror("Error: Cannot create text \"%s\"\n", str);
                    abort();
                }

                RsvgHandle *h = getSVGHandler(id);
                ret.tl = addTextList(h, t -> p, va_arg(args, struct textList*));

                free(id);
                free(str);
            }
            break;
        case AST_RGBA:
            {
                struct _ast_rgba *t = (struct _ast_rgba*) a;
                ret.c = malloc(sizeof(struct color));
                ret.c -> r = eval(t -> r).d;
                ret.c -> g = eval(t -> g).d;
                ret.c -> b = eval(t -> b).d;
                ret.c -> a = eval(t -> a).d;
            }
            break;
        case AST_TEXT:
            {
                ret.s = ((struct _ast_term*) a) -> val.s;
            }
            break;
        case AST_INTL:
            {
                ret.i = ((struct _ast_term*) a) -> val.i;
            }
            break;
        case AST_DRAW:
            {
                struct primitive *p = eval(a -> l).p;
                preparePrimitive(p);
                ret.e = newDrawEvent(p);
            }
            break;
        case AST_DIR:
            {
                setDirection( ((struct _ast_term*) a ) -> val.i );
            }
            break;
        case AST_KF:
            {
                struct _ast_kf *t = (struct _ast_kf*) a;
                struct ast *d = t -> duration;
                if (d) {
                    newKeyframe(eval(d).d, t -> easingFunc);
                } else {
                    newKeyframe(1.0, t -> easingFunc);
                }
            }
            break;
        case AST_TBL:
            {
                struct symTable *tb;
                if (!(tb =  eval(a -> l).e -> a.pr -> tb)) {
                    fprintf(stderr, "Error: primitive is not a block\n");
                    abort();
                }
                tb = switchTable(tb);

                ret.e = eval(a -> r).e;

                switchTable(tb);
            }
            break;
        case AST_LBL:
            {
                char *sym = ((struct _ast_term*) a) -> val.s;
                if (!(s = lookup(sym))) {
                    fprintf(stderr, "Error: label \"%s\" "
                            "not found\n", sym);
                    abort();
                }
                ret.e = s -> val.e;
            }
            break;
        case AST_ORD:
            {
                struct _ast_ord *t = (struct _ast_ord*) a;
                if (t -> r) {
                    if (!(ret.e = getPrim_r(t -> T, t -> i))) {
                        fprintf(stderr, "Error: %dth primitive "
                                "out of range\n", t -> i);
                        abort();
                    }
                } else {
                    if (!(ret.e = getPrim(t -> T, t -> i))) {
                        fprintf(stderr, "Error: %dth last primitive "
                                "out of range\n", t -> i);
                        abort();
                    }
                }
            }
            break;
        case AST_ASGN:
            {
                struct _ast_asgn *t = (struct _ast_asgn*) a;
                union T v;
                if (t -> varT == SYM_DOUBLE) {
                    ret.d = v.d = eval(t -> a).d;
                    setSym(t -> sym, SYM_DOUBLE, v);
                } else {
                    ret.e = v.e = eval(t -> a).e;
                    setSym(t -> sym, SYM_EVENT, v);
                }
            }
            break;
        case AST_RST:
            {
                struct _ast_rst *t = (struct _ast_rst*) a;
                if (t -> tl) {
                    struct _text_list *tl = t -> tl;
                    while (tl) {
                        resetSym(tl -> s);
                        tl = tl -> next;
                    }
                } else {
                    clearSym();
                }
            }
            break;
        case AST_PRN:
            {
                fprintf(stderr, "%g\n", eval(a -> l).d);
            }
            break;

        default:
            yyerror("Internal Error: cannot evaluate "
                    "AST with type of %d\n", a -> t);
            abort();
            break;
    }

    va_end(args);

    return ret;
}

void freeTree (struct ast* a) {
    if (!a) return;

    switch (a -> t) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '^':
        case AST_UNM:
        case AST_SIN:
        case AST_COS:
        case AST_ATAN2:
        case AST_LOG:
        case AST_EXP:
        case AST_SQRT:
        case AST_MAX:
        case AST_MIN:
        case AST_INT:
        case AST_RAND:
        case AST_ABS:
        case '!':
        case '<':
        case '>':
        case AST_EE:
        case AST_NE:
        case AST_LE:
        case AST_GE:
        case AST_OR:
        case AST_AND:
        case AST_XOR:
        case AST_SAME:
        case AST_STMT:
        case AST_GRP:
        case AST_RPT:
        case AST_DRAW:
        case AST_PRN:
        case AST_TBL:
        case AST_VEC:
        case AST_VADD:
        case AST_VSUB:
        case AST_VSEP:
            freeTree(a -> l);
            freeTree(a -> r);
            break;

        case AST_HERE:
        case AST_DIR:
        case AST_NUM:
        case AST_TEXT:
        case AST_INTL:
        case AST_PRIM:
        case AST_ORD:
            break;

        case AST_REF:
        case AST_LBL:
            {
                struct _ast_term *t;
                t = (struct _ast_term*) a;
                free(t -> val.s);
            }
            break;
        case AST_RST:
            {
                struct _ast_rst *t = (struct _ast_rst*) a;
                struct _text_list *tl = t -> tl;
                while (tl) {
                    struct _text_list *tl_0 = tl;
                    tl = tl -> next;
                    free(tl_0);
                }
            }
            break;
        case AST_KF:
            {
                struct _ast_kf *t = (struct _ast_kf*) a;
                freeTree(t -> duration);
            }
            break;
        case AST_ASGN:
            {
                struct _ast_asgn *t = (struct _ast_asgn*) a;
                freeTree(t -> a);
                free(t -> sym);
            }
            break;
        case AST_ATTR:
            {
                struct _ast_attr *t = (struct _ast_attr*) a;
                freeTree(t -> p);
                freeTree(t -> val);
            }
            break;
        case AST_TL:
            {
                struct _ast_tl *t = (struct _ast_tl*) a;
                freeTree(t -> s);
            }
            break;
        case AST_RGBA:
            {
                struct _ast_rgba *t = (struct _ast_rgba*) a;
                freeTree(t -> r);
                freeTree(t -> g);
                freeTree(t -> b);
                freeTree(t -> a);
            }
            break;
        case AST_BY:
        case AST_LOC:
            {
                struct _ast_ai *t = (struct _ast_ai*) a;
                freeTree(t -> a);
            }
            break;
        case AST_VBET:
            {
                struct _ast_vbet *t = (struct _ast_vbet*) a;
                freeTree(t -> p);
                freeTree(t -> s);
                freeTree(t -> e);
            }
            break;
        case AST_FOR:
            {
                struct _ast_for *t = (struct _ast_for*) a;
                freeTree(t -> init);
                freeTree(t -> term);
                freeTree(t -> inc);
                freeTree(t -> body);
            }
            break;

        // default:
            yyerror("Internal Error: cannot free "
                    "AST with type of %d\n", a -> t);
            abort();
            break;
    }
    free(a);
}
