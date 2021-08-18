#include "ast.h"
#include "tex.h"
#include "present.h"
#include "symtable.h"
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

struct _ast_tl {
    int t;
    struct ast* l;
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

struct _ast_one {
    int t;
    struct ast *a;
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
astTL(struct ast *l, struct ast *s, int p) {
    struct _ast_tl *a = malloc(sizeof(struct _ast_tl));
    a -> t = AST_TL;
    a -> l = l;
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
    struct _ast_one *a = malloc(sizeof(struct _ast_one));
    a -> t = AST_DRAW;
    a -> a = l;
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
astRef(char *s) {
    struct _ast_term *a = malloc(sizeof(struct _ast_term));
    a -> t = AST_REF; 
    a -> val.s = s;
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
            p -> txt = eval(a -> val).tl;
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
    }

    return p;
}

union ast_type
eval(struct ast *a) {
    if (!a) return (union ast_type) {.i = 0};
    union ast_type ret;
    struct symbol *s;

    switch (a -> t) {
        /* expresions */
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
            if (t_tl -> l) {
                ret.tl = addTextList(h, t_tl -> p, eval(t_tl -> l).tl);
            } else {
                ret.tl = addTextList(h, t_tl -> p, NULL);
            }

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
        case AST_DRAW:
            struct primitive *p = eval(
                ((struct _ast_one*) a ) -> a
            ).p;
            preparePrimitive(p);
            newDrawEvent(p);
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

        default:
            yyerror("Internal error unknown AST type %d\n", a -> t);
            abort();
            break;
    }

    return ret;
}
