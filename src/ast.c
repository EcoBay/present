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
    return (struct ast*) a;
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
astLoc(char *sym, int corner) {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = 0;

    a -> l = malloc(sizeof(struct ast*));
    a -> l -> t = 1;
    a -> l -> l = astLbl(sym);
    a -> l -> r = astInt(corner);

    a -> r = malloc(sizeof(struct ast*));
    a -> r -> t = 2;
    a -> r -> l = astLbl(sym);
    a -> r -> r = astInt(corner);

    return (struct ast*) a;
}

struct ast*
astHere() {
    struct ast *a = malloc(sizeof(struct ast));
    a -> t = 0;

    a -> l = malloc(sizeof(struct ast*));
    a -> l -> t = 3;

    a -> r = malloc(sizeof(struct ast*));
    a -> r -> t = 4;
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
astLbl(char *s) {
    struct _ast_term *a = malloc(sizeof(struct _ast_term));
    a -> t = AST_LBL; 
    a -> val.s = s;
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
    return (struct ast*) a;
}

static struct primitive*
evalPrim(struct _ast_prim *a) {
    struct primitive *p = newPrimitive(a -> pt);
    struct symbol *s;
    uint8_t direction = getDirection();

    switch (a -> pt) {
        case PRIM_BOX:
            GET_FLOAT_SYM(s , "boxht");
            p -> ht = s -> val.d;
            GET_FLOAT_SYM(s , "boxwid");
            p -> wid = s -> val.d;
            break;
        case PRIM_CIRCLE:
            GET_FLOAT_SYM(s, "circlerad");
            p -> expr = s -> val.d;
            break;
        case PRIM_ELLIPSE:
            GET_FLOAT_SYM(s, "ellipseht");
            p -> ht = s -> val.d;
            GET_FLOAT_SYM(s, "ellipsewid");
            p -> wid = s -> val.d;
            break;
        case PRIM_ARC:
            GET_FLOAT_SYM(s, "arcrad");
            p -> expr = s -> val.d;
            break;
        case PRIM_ARROW:
            p -> arrowStyle = 1;
        case PRIM_LINE:
        case PRIM_SPLINE:
            switch (direction) {
                case 0:
                case 2:
                    GET_FLOAT_SYM(s, "lineht");
                    break;
                case 1:
                case 3:
                    GET_FLOAT_SYM(s, "linewid");
                    break;
            }
            p -> expr = s -> val.d;
            break;
        case PRIM_MOVE:
            switch (direction) {
                case 0:
                case 2:
                    GET_FLOAT_SYM(s, "moveht");
                    break;
                case 1:
                case 3:
                    GET_FLOAT_SYM(s, "movewid");
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
            p -> txt = eval(a -> val, p -> txt).tl;
            break;
        case ATTR_CH:
            pushTable();
            g_parent = p;
            eval(a -> val);
            p -> tb = popTable();
            g_parent = NULL;
            break;

        case ATTR_UP:
            if (p -> t > 3 && p -> t < 8) {
                struct location *l;
                l = getLastSegment(p);

                if (a -> val) {
                    l -> y += eval(a -> val).d;
                } else {
                    if (p -> t == PRIM_MOVE) {
                        GET_FLOAT_SYM(s, "moveht");
                    } else {
                        GET_FLOAT_SYM(s, "lineht");
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
                        GET_FLOAT_SYM(s, "movewid");
                    } else {
                        GET_FLOAT_SYM(s, "linewid");
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
                        GET_FLOAT_SYM(s, "moveht");
                    } else {
                        GET_FLOAT_SYM(s, "lineht");
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
                        GET_FLOAT_SYM(s, "movewid");
                    } else {
                        GET_FLOAT_SYM(s, "linewid");
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
                l = getLastSegment(p);
                l -> x += eval(a -> val -> l).d;
                l -> y += eval(a -> val -> r).d;

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
            p -> arrowStyle &= ~3;
            p -> arrowStyle |= 2;
            break;
        case ATTR_RARROW:
            p -> arrowStyle &= ~3;
            p -> arrowStyle |= 1;
            break;
        case ATTR_LRARROW:
            p -> arrowStyle |= 3;
            break;
        case ATTR_CW:
            p -> flags |= 2;
            break;
        case ATTR_DASHED:
            p -> flags &= ~12;
            p -> flags |= 4;

            if (a -> val) {
                p -> spacing = eval(a -> val).d;
            } else {
                p -> spacing = 0.05;
            }
            break;
        case ATTR_DOTTED:
            p -> flags &= ~12;
            p -> flags |= 8;

            if (a -> val) {
                p -> spacing = eval(a -> val).d;
            } else {
                p -> spacing = 0.05;
            }
            break;
        case ATTR_SOLID:
            p -> flags &= ~16;
            break;
        case ATTR_INVIS:
            p -> flags |= 16;
            break;
        case ATTR_FILL:
            p -> flags |= 32;
            p -> fill = eval(a -> val).c;
            break;
        case ATTR_HT:
            p -> ht = eval(a -> val).d;
            break;
        case ATTR_WID:
            p -> wid = eval(a -> val).d;
            break;
        case ATTR_RAD:
            p -> rad = eval(a -> val).d;
            p -> flags |= 64;
            break;
        case ATTR_DIAM:
            p -> rad = eval(a -> val).d / 2.0;
            p -> flags |= 64;
            break;
        case ATTR_EXPR:
            p -> expr = eval(a -> val).d / 2.0;
            break;
        case ATTR_FROM:
            p -> start.x = eval(a -> val -> l).d;
            p -> start.y = eval(a -> val -> r).d;
            break;
        case ATTR_TO:
            if (p -> t > 2 && p -> t < 8) {
                struct location *l;
                l = getLastSegment(p);
                l -> x = eval(a -> val -> l).d;
                l -> y = eval(a -> val -> r).d;

                p -> flags |= 1;
            }
            break;
        case ATTR_CHOP:
            if (p -> t > 3 && p -> t < 8) {
                if (p -> flags & 128) {
                    p -> chop2 = eval(a -> val).d;
                } else {
                    p -> chop1 = p -> chop2 = eval(a -> val).d;
                    p -> flags |= 128;
                }
            }
            break;
        case ATTR_WITH:
            p -> with = eval(a -> val).i;
            break;
        case ATTR_AT:
            if (!p -> at) {
                p -> at = malloc(sizeof(struct vec2d));
            }

            p -> at -> x = eval(a -> val -> l).d;
            p -> at -> y = eval(a -> val -> r).d;
            break;
    }

    return p;
}

union ast_type
eval(struct ast *a, ...) {
    if (!a) return (union ast_type) {.i = 0};
    union ast_type ret;
    struct symbol *s;

    va_list args;
    va_start(args, a);

    switch (a -> t) {
        /* expresions */
        case 1:
            {
                struct vec2d *e = malloc(sizeof(struct vec2d));
                struct primitive *p = eval(a -> l).e -> a.pr;
                getLoc(p, e, eval(a -> r).i);
                ret.d = e -> x;
                free(e);
            }
            break;
        case 2:
            {
                struct vec2d *e = malloc(sizeof(struct vec2d));
                struct primitive *p = eval(a -> l).e -> a.pr;
                getLoc(p, e, eval(a -> r).i);
                ret.d = e -> y;
                free(e);
            }
            break;
        case 3:
            {
                struct vec2d *e = malloc(sizeof(struct vec2d));
                getCursor(e);
                ret.d = e -> x;
                free(e);
            }
            break;
        case 4:
            {
                struct vec2d *e = malloc(sizeof(struct vec2d));
                getCursor(e);
                ret.d = e -> y;
                free(e);
            }
            break;
        case AST_NUM:
            ret.d = ((struct _ast_term*) a) -> val.d;
            break;
        case AST_REF:
            GET_FLOAT_SYM(s, ((struct _ast_term*) a) -> val.s);
            ret.d = s -> val.d;
            break;
        case '+':
            ret.d = eval(a -> l).d + eval(a -> r).d;
            break;
        case '-':
            ret.d = eval(a -> l).d + eval(a -> r).d;
            break;
        case '*':
            ret.d = eval(a -> l).d * eval(a -> r).d;
            break;
        case '/':
            float divisor = eval(a -> r).d;
            if (divisor == 0.0) {
                yyerror("Error: division by zero\n");
                abort();
            }
            ret.d = eval(a -> l).d / divisor;
            break;
        case '%':
            float modulo = eval(a -> r).d;
            if (modulo == 0.0) {
                yyerror("Error: modulos by zero\n");
                abort();
            }
            ret.d = fmodf(eval(a -> l).d, modulo);
            break;
        case '^':
            errno = 0;
            ret.d = pow(eval(a -> l).d, eval(a -> r).d);
            if (errno == EDOM) {
                yyerror("Error: arguments to '&' is out of domain\n");
                abort();
            }
            break;
        case '!':
            ret.d = (eval(a -> l).d == 0.0);
            break;
        case AST_UNM:
            ret.d = - eval(a -> l).d;
            break;
        case AST_SIN:
            errno = 0;
            ret.d = sin(eval(a -> l).d);
            if (errno == ERANGE) {
                yyerror("Error: sin result out of range\n");
                abort();
            }
            break;
        case AST_COS:
            errno = 0;
            ret.d = cos(eval(a -> l).d);
            if (errno == ERANGE) {
                yyerror("Error: cos result out of range\n");
                abort();
            }
            break;
        case AST_ATAN2:
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
            break;
        case AST_LOG:
            errno = 0;
            ret.d = log(eval(a -> l).d);
            if (errno == ERANGE) {
                yyerror("Error: log result out of range\n");
                abort();
            }
            break;
        case AST_EXP:
            errno = 0;
            ret.d = pow(10.0, eval(a -> l).d);
            if (errno == ERANGE) {
                yyerror("Error: exp result out of range\n");
                abort();
            }
            break;
        case AST_SQRT:
            errno = 0;
            ret.d = sqrt(eval(a -> l).d);
            if (errno == EDOM) {
                yyerror("error: sqrt result out of range\n");
                abort();
            }
            break;
        case AST_MAX:
            ret.d = MAX(eval(a -> l).d, eval(a -> r).d);
            break;
        case AST_MIN:
            ret.d = MIN(eval(a -> l).d, eval(a -> r).d);
            break;
        case AST_INT:
            ret.d = INT(eval(a -> l).d);
            break;
        case AST_RAND:
            ret.d = (float) rand() / (float) RAND_MAX;
            break;
        case AST_ABS:
            ret.d = fabsf(eval(a -> l).d);
            break;

        case AST_STMT:
            eval(a -> l);
            eval(a -> r);
            break;
        case AST_GRP:
            struct vec2d c;
            getCursor(&c);
            int dir = getDirection();
            pushTable();

            eval(a -> l);

            freeTable(popTable());
            setDirection(dir);
            setCursor(&c);
            break;
        case AST_PRIM:
            ret.p = evalPrim( (struct _ast_prim*) a );
            break;
        case AST_ATTR:
            ret.p = evalAttr( (struct _ast_attr*) a );
            break;
        case AST_TL:
            GET_FLOAT_SYM(s, "ps");
            struct _ast_tl *t_tl = (struct _ast_tl*) a;
            char *str = eval(t_tl -> s).s;
            char *id = createTex(str, s -> val.d);
            if (tex2SVG(id)) {
                yyerror("Error: Cannot create text \"%s\"\n", str);
                abort();
            }

            RsvgHandle *h = getSVGHandler(id);
            ret.tl = addTextList(h, t_tl -> p, va_arg(args, struct textList*));

            free(id);
            free(str);
            break;
        case AST_RGBA:
            struct _ast_rgba *t_rgba = (struct _ast_rgba*) a;
            ret.c = malloc(sizeof(struct color));
            ret.c -> r = eval(t_rgba -> r).d;
            ret.c -> g = eval(t_rgba -> g).d;
            ret.c -> b = eval(t_rgba -> b).d;
            ret.c -> a = eval(t_rgba -> a).d;
            break;
        case AST_TEXT:
            ret.s = ((struct _ast_term*) a) -> val.s;
            break;
        case AST_INTL:
            ret.i = ((struct _ast_term*) a) -> val.i;
            break;
        case AST_DRAW:
            struct primitive *p = eval(a -> l).p;
            preparePrimitive(p);
            ret.e = newDrawEvent(p);
            break;
        case AST_DIR:
            setDirection( ((struct _ast_term*) a ) -> val.i );
            break;
        case AST_KF:
            struct _ast_kf *t_kf = (struct _ast_kf*) a;
            struct ast *d = t_kf -> duration;
            if (d) {
                newKeyframe(eval(d).d, t_kf -> easingFunc);
            } else {
                newKeyframe(1.0, t_kf -> easingFunc);
            }
            break;
        case AST_LBL:
            GET_EVENT_SYM(s, ((struct _ast_term*) a) -> val.s);
            ret.e = s -> val.e;
            break;
        case AST_ASGN:
            struct _ast_asgn *t_asgn = (struct _ast_asgn*) a;
            union T v;
            if (t_asgn -> varT == SYM_DOUBLE) {
                v.d = eval(t_asgn -> a).d;
                setSym(t_asgn -> sym, SYM_DOUBLE, v);
            } else {
                v.e = eval(t_asgn -> a).e;
                setSym(t_asgn -> sym, SYM_EVENT, v);
            }
            break;
        case AST_RST:
            struct _ast_rst *t_rst = (struct _ast_rst*) a;
            if (t_rst -> tl) {
                struct _text_list *tl = t_rst -> tl;
                while (tl) {
                    resetSym(tl -> s);
                    tl = tl -> next;
                }
            } else {
                clearSym();
            }
            break;
        case AST_PRN:
            printf("%g\n", eval(a -> l).d);
            break;

        default:
            yyerror("Internal error unknown AST type %d\n", a -> t);
            abort();
            break;
    }

    va_end(args);

    return ret;
}

void freeTree (struct ast* a) {
    if (!a) return;

    switch (a -> t) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '^':
        case '!':
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
        case AST_STMT:
        case AST_GRP:
        case AST_DRAW:
        case AST_PRN:
            freeTree(a -> l);
            freeTree(a -> r);
            break;

        case AST_DIR:
        case AST_LBL:
        case AST_NUM:
        case AST_REF:
        case AST_TEXT:
        case AST_INTL:
        case AST_RST:
        case AST_PRIM:
            break;

        case AST_KF:
            struct _ast_kf *t_kf = (struct _ast_kf*) a;
            freeTree(t_kf -> duration);
            break;
        case AST_ASGN:
            struct _ast_asgn *t_asgn = (struct _ast_asgn*) a;
            freeTree(t_asgn -> a);
            break;
        case AST_ATTR:
            struct _ast_attr *t_attr = (struct _ast_attr*) a;
            freeTree(t_attr -> p);
            freeTree(t_attr -> val);
            break;
        case AST_TL:
            struct _ast_tl *t_tl = (struct _ast_tl*) a;
            freeTree(t_tl -> s);
            break;
        case AST_RGBA:
            struct _ast_rgba *t_rgba = (struct _ast_rgba*) a;
            freeTree(t_rgba -> r);
            freeTree(t_rgba -> g);
            freeTree(t_rgba -> b);
            freeTree(t_rgba -> a);
            break;

        default:
            yyerror("Internal error unknown AST type %d\n", a -> t);
            abort();
            break;
    }
}
