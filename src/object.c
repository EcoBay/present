#include "tex.h"
#include "draw.h"
#include "object.h"
#include "present.h"
#include "symtable.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct presentation *g_presentation = NULL;
struct primitive *g_parent = NULL;

void*
getLast(void *i){
    if (!i) return NULL;

    struct linkedList{
        struct linkedList *next;
    } *l = i;

    if ((l -> next)) return getLast(l -> next);
    else return i;
}

void
newScene(){
    struct scene *s = malloc(sizeof(struct scene));
    s -> next = NULL;
    s -> keyframes = NULL;
    s -> cursor = (struct vec2d) {-4.75, 3.25};
    s -> direction = 1; // Right

    if (!g_presentation -> scenes) g_presentation -> scenes = s;
    else {
        struct scene *t = g_presentation -> scenes;
        t = getLast(t);
        t -> next = s;
        popTable();
    }

    pushTable();
}

void
newKeyframe(float duration, enum easingFunction easingFunc){
    struct keyframe *k = malloc(sizeof(struct keyframe));
    k -> next = NULL;
    k -> events = NULL;
    k -> duration = duration;
    k -> easingFunc = easingFunc;

    if(!g_presentation -> scenes) {
        fprintf(stderr, "Warning: adding default scene before "
                "line no. %d\n", yylineno);
        newScene();
    }


    struct scene *s = getLast(g_presentation -> scenes);
    if (!s -> keyframes) s -> keyframes = k;
    else {
        popTable();
        struct keyframe *t = s -> keyframes;
        t = getLast(t);
        t -> next = k;
    }

    pushTable();
}

struct event*
newDrawEvent(struct primitive *p){
    struct scene *s = g_presentation -> scenes;
    s = getLast(s);

    struct event *e = malloc(sizeof(struct event));
    e -> next = NULL;
    e -> eventType = 0;
    e -> a.pr = p;

    if (g_parent) {

        struct event *c = getLast(g_parent -> child);
        if (!c) {
            g_parent -> child = e;
        } else {
            c -> next = e;
        }

    } else {

        if (!s -> keyframes) {
            fprintf(stderr, "Warning: adding default keyframe before "
                    "line no. %d\n", yylineno - 1);
            newKeyframe(1.0, EASE_LINEAR);
        }

        struct keyframe *k = getLast(s -> keyframes);
        if (!k -> events) {
            k -> events = e;
        } else {
            struct event *t = k -> events;
            t = getLast(t);
            t -> next = e;
        }
    }

    return e;
}

struct primitive*
newPrimitive(enum primitiveType t){
    struct primitive *p = malloc(sizeof(struct primitive));
    p -> t = t;
    p -> expr = 0;
    p -> ht = 0.5;
    p -> wid = 0.75;
    p -> rad = 0.25;
    p -> segments = NULL;
    p -> at = NULL;
    p -> with = (t == PRIM_TEXT_LIST) ? 12 : 0;
    p -> spacing = 0;
    p -> chop1 = 0;
    p -> chop2 = 0;
    p -> arrowStyle = 0;
    p -> flags = 0;
    p -> fill = NULL;
    p -> txt = NULL;

    p -> tb = NULL;
    p -> child = NULL;

    struct symbol *s;
    s = lookup("ps");
    p -> ps = s -> val.d;
    s = lookup("arrowht");
    p -> arrowht = s -> val.d;
    s = lookup("arrowwid");
    p -> arrowwid = s -> val.d;
    s = lookup("arrowhead");
    p -> arrowhead = (uint8_t) s -> val.d;

    return p;
}

static void
chopBoundingBox(struct primitive *p){
    switch (p -> t) {
        default:
            return;
        case PRIM_CIRCLE:
        case PRIM_ELLIPSE:
        case PRIM_ARC:
    }

    float wid = p -> wid / 2, ht = p -> ht / 2;

    float rad = (p -> flags & 64) ? p -> rad : p -> expr;
    if(p -> t == PRIM_CIRCLE) wid = ht = rad;

    p -> ne.y -= ht  * 0.2928932188;
    p -> ne.x -= wid * 0.2928932188;
    p -> sw.y += ht  * 0.2928932188;
    p -> sw.x += wid * 0.2928932188;
    p -> se.y += ht  * 0.2928932188;
    p -> se.x -= wid * 0.2928932188;
    p -> nw.y -= ht  * 0.2928932188;
    p -> nw.x += wid * 0.2928932188;

}

static void
chopLine(struct primitive *p) {
    // TODO: Allow for chopping arc (not part of standard pic)
    if (p -> t < 4 || p -> t > 7) return;

    struct vec2d l0 = { p -> start.x, p -> start.y };
    struct location *l = p -> segments;

#define SQ(a) ((a) * (a))
    float C = sqrt(SQ(l0.x - l -> x)
            + SQ(l0.y - l -> y));
    float q = p -> chop1 / C;

    p -> start.x += (l -> x - l0.x) * q;
    p -> start.y += (l -> y - l0.y) * q;

    while (l -> next) {
        l0.x = l -> x;
        l0.y = l -> y;
        l = l -> next;
    }

    C = sqrt(SQ(l0.x - l -> x)
      + SQ(l0.y - l -> y));
    q = p -> chop2 / C;

    l -> x -= (l -> x - l0.x) * q;
    l -> y -= (l -> y - l0.y) * q;
#undef SQ

}

static void
updateBoundingBox(struct primitive *p, struct vec2d *ps, uint8_t c){
    struct vec2d min = ps[0];
    struct vec2d max = ps[0];
    for(int i = 1; i < c; i++){
        if(ps[i].x < min.x) min.x = ps[i].x;
        else if(ps[i].x > max.x) max.x = ps[i].x;
        if(ps[i].y < min.y) min.y = ps[i].y;
        else if(ps[i].y > max.y) max.y = ps[i].y;
    }

    struct vec2d mid = {
        (min.x + max.x) / 2,
        (min.y + max.y) / 2
    };

    p -> sw = min;
    p -> ne = max;
    p -> c  = mid;

    p -> nw = (struct vec2d) { min.x, max.y };
    p -> se = (struct vec2d) { max.x, min.y };

    p -> n  = (struct vec2d) { mid.x, max.y };
    p -> e  = (struct vec2d) { max.x, mid.y };
    p -> w  = (struct vec2d) { min.x, mid.y };
    p -> s  = (struct vec2d) { mid.x, min.y };
}

static void
prepareTextList(struct textList *t, struct vec2d *o, struct vec2d **ps, uint8_t *count){
    int c = 0;
    for (struct textList *to = t; to; to = to -> next, c++);

    struct symbol *s;
    s = lookup("vs");
    float ht = s -> val.d / DPI;
    float curY = o -> y + ht * (c-1) / 2;

    if (ps && count) {
        *ps = malloc(2 * c * sizeof(struct vec2d));
        *count = 2 * c;
    }

    for (int i = 0; i < c; i++) {
        struct vec2d dim;
        getSVGDim(t -> h, &dim);

        dim.x /= DPI;
        dim.y /= DPI;

        t -> nw.x = o -> x - dim.x / 2;
        t -> nw.y = curY + dim.y / 2;

        if (t -> positioning & 1) {
            t -> nw.x += dim.x / 2;
        } //ljust
        else if (t -> positioning & 2) {
            t -> nw.x -= dim.x / 2;
        } //rjust

        if (t -> positioning & 4) {
            t -> nw.y += dim.y / 2;
        } //above
        else if (t -> positioning & 8) {
            t -> nw.y -= dim.y / 2;
        } //below

        if (ps && count) {
            (*ps)[i * 2] = t -> nw;
            (*ps)[i * 2 + 1].x = t -> nw.x + dim.x;
            (*ps)[i * 2 + 1].y = t -> nw.y - dim.y;
        }

        curY -= ht;
        t = t -> next;
    }
}

static void
translatePrimitive(struct primitive *p) {
    struct vec2d ref;
    struct vec2d trl;

    switch (p -> with) {
        case 0: ref = p -> c; break;

        case 1: ref = p -> n; break;
        case 4: ref = p -> e; break;
        case 8: ref = p -> w; break;
        case 2: ref = p -> s; break;

        case 5: ref = p -> ne; break;
        case 9: ref = p -> nw; break;
        case 6: ref = p -> se; break;
        case 10: ref = p -> sw; break;

        case 3: ref = p -> end; break;
        case 12: ref = p -> start; break;
    }

    trl.x = p -> at -> x - ref.x;
    trl.y = p -> at -> y - ref.y;

#define INC_VEC(A, B) A.x += B.x; \
    A.y += B.y
    
    INC_VEC(p -> n, trl);
    INC_VEC(p -> e, trl);
    INC_VEC(p -> w, trl);
    INC_VEC(p -> s, trl);
    INC_VEC(p -> c, trl);

    INC_VEC(p -> nw, trl);
    INC_VEC(p -> ne, trl);
    INC_VEC(p -> sw, trl);
    INC_VEC(p -> se, trl);

    INC_VEC(p -> start, trl);
    INC_VEC(p -> end, trl);


    struct textList *t = p -> txt;
    while (t) {
        INC_VEC(t -> nw, trl);
        t = t -> next;
    }

    struct location *l = p -> segments;
    while (l) {
        l -> x += trl.x;
        l -> y += trl.y;
        l = l -> next;
    }

    struct event *e = p -> child;
    while (e) {
        struct primitive *t = e -> a.pr;
        if (!t -> at) {
            t -> at = malloc(sizeof(struct vec2d));
        }

        t -> at -> x = t -> start.x + trl.x;
        t -> at -> y = t -> start.y + trl.y;
        t -> with = 12;
        translatePrimitive(t);

        e = e -> next;
    }
}

void
preparePrimitive(struct primitive *p){
    struct location *l;
    struct vec2d *ps;
    uint8_t count;
    float rad = (p -> flags & 64) ? p -> rad : p -> expr;

    switch (p -> t) {
        case PRIM_BOX:
        case PRIM_ELLIPSE:
        case PRIM_CIRCLE:
            float wid = (p -> t == PRIM_CIRCLE) ?
                rad * 2 : p -> wid;
            float ht  = (p -> t == PRIM_CIRCLE) ?
                rad * 2 : p -> ht;

            ps = malloc(4 * sizeof(struct vec2d));
            count = 4;

            if (p -> direction == 2) {
                ht *= -1.0;
            } else if (p -> direction == 3) {
                wid *= -1.0;
            }

            switch (p -> direction) {
                case 0:
                case 2:
                    ps[0] = (struct vec2d) {
                        p -> start.x,
                        p -> start.y
                    };
                    ps[1] = (struct vec2d) {
                        p -> start.x,
                        p -> start.y + ht,
                    };
                    ps[2] = (struct vec2d) {
                        p -> start.x - wid / 2,
                        p -> start.y + ht  / 2,
                    };
                    ps[3] = (struct vec2d) {
                        p -> start.x + wid / 2,
                        p -> start.y + ht  / 2,
                    };
                    break;
                case 1:
                case 3:
                    ps[0] = (struct vec2d) {
                        p -> start.x,
                        p -> start.y
                    };
                    ps[1] = (struct vec2d) {
                        p -> start.x + wid,
                        p -> start.y,
                    };
                    ps[2] = (struct vec2d) {
                        p -> start.x + wid / 2,
                        p -> start.y - ht  / 2,
                    };
                    ps[3] = (struct vec2d) {
                        p -> start.x + wid / 2,
                        p -> start.y + ht  / 2,
                    };
                    break;
            }
            p -> end = ps[1];
            break;
        case PRIM_LINE:
        case PRIM_ARROW:
        case PRIM_SPLINE:
        case PRIM_MOVE:
            l = getLastSegment(p);
            if (!(p -> flags & 1)){
                float e = p -> expr;
                switch (p -> direction) {
                    case 0: l -> y += e; break;
                    case 1: l -> x += e; break;
                    case 2: l -> y -= e; break;
                    case 3: l -> x -= e; break;
                }
            };

            count = 1;
            for(l = p -> segments; l; l = l -> next, count++);

            ps = malloc(count * sizeof(struct vec2d));
            ps[0] = p -> start;

            l = p -> segments;
            for(int i = 1; i < count; i++){
                ps[i] = (struct vec2d) {
                    l -> x, l -> y
                };
                l = l -> next;
            }
            p -> end = ps[count - 1];
            break;
        case PRIM_ARC:
            if (p -> flags & 1) { 
                ps = malloc(2 * sizeof(struct vec2d));
                count = 2;

                float x0 = p -> start.x;
                float x1 = p -> segments -> x;
                float y0 = p -> start.y;
                float y1 = p -> segments -> y;

#define SQ(a) ((a) * (a))
                float q = sqrt(SQ(x0 - x1) + SQ(y0 - y1));
                rad = MAX(q / 2.0, rad);

                float x2 = ( x1 + x0 ) / 2.0;
                float y2 = ( y1 + y0 ) / 2.0;

                float cw = p -> flags & 2 ? -1 : 1;
                float x3 = x2 + sqrt(SQ(rad)-SQ(q/2))*(y0-y1)/q*cw;
                float y3 = y2 + sqrt(SQ(rad)-SQ(q/2))*(x1-x0)/q*cw;
#undef SQ

                ps[0].x = x3 + rad;
                ps[0].y = y3 + rad;
                ps[1].x = x3 - rad;
                ps[1].y = y3 - rad;

                p -> end.x = x1;
                p -> end.y = y1;
                p -> rad = rad;
                p -> flags |= 64;

            } else {
                ps = malloc(4 * sizeof(struct vec2d));
                count = 4;
                uint8_t dir = (p -> flags & 2) ? 1 : 3;
                dir = (dir + p -> direction) % 4;

                float invY = 1.0, invX = 1.0;
                if (dir == 2 || dir == 1) invY = -1.0;
                if (dir == 3 || dir == 2) invX = -1.0;

                switch (dir) {
                    case 0:
                    case 2:
                        ps[0] = (struct vec2d) {
                            p -> start.x,
                            p -> start.y
                        };
                        ps[1] = (struct vec2d) {
                            p -> start.x,
                            p -> start.y + rad * invX * 2,
                        };
                        ps[2] = (struct vec2d) {
                            p -> start.x - rad * invX,
                            p -> start.y + rad * invY,
                        };
                        ps[3] = (struct vec2d) {
                            p -> start.x + rad * invX,
                            p -> start.y + rad * invY,
                        };
                        break;
                    case 1:
                    case 3:
                        ps[0] = (struct vec2d) {
                            p -> start.x,
                            p -> start.y
                        };
                        ps[1] = (struct vec2d) {
                            p -> start.x + rad * invX * 2,
                            p -> start.y,
                        };
                        ps[2] = (struct vec2d) {
                            p -> start.x + rad * invX,
                            p -> start.y - rad * invY,
                        };
                        ps[3] = (struct vec2d) {
                            p -> start.x + rad * invX,
                            p -> start.y + rad * invY,
                        };
                        break;
                }

                p -> end = (p -> flags & 2) ? ps[2] : ps[3];
                setDirection(dir);
            }
            break;
        case PRIM_TEXT_LIST:
            prepareTextList(p -> txt, &p -> start, &ps, &count);
            p -> end = p -> start;
            break;
        case PRIM_BLOCK:
            count = 0;
            struct event *e = p -> child;
            for(; e; e = e -> next, count+=2);
            ps = malloc(count * sizeof(struct vec2d));

            e = p -> child;
            for(int i = 0; e; e = e -> next, i++) {
                ps[i * 2] = e -> a.pr -> nw;
                ps[i * 2 + 1] = e -> a.pr -> se;
                p -> end = e -> a.pr -> end;
            }

            break;
    }

    updateBoundingBox(p, ps, count);
    if (p -> t < PRIM_TEXT_LIST || p -> t == PRIM_BLOCK) {
        prepareTextList(p -> txt, &p -> c, NULL, NULL);
    }

    chopBoundingBox(p);
    chopLine(p);

    if (p -> at) {
        translatePrimitive(p);
    }

    free(ps);
    setCursor(&p -> end);
}

void
initPresentation(){
    g_presentation = malloc(sizeof(struct presentation));
    g_presentation -> scenes = NULL;
}

void
setDirection(uint8_t d){
    struct scene *s = getLast(g_presentation -> scenes);
    if (s) {
        s -> direction = d;
    } else {
        fprintf(stderr, "Warning: start scene first "
                "before setting direction. Disregrading "
                "command at line no %d\n", yylineno);
    }
}

uint8_t
getDirection(){
    struct scene *s = getLast(g_presentation -> scenes);
    if (!s) {
        fprintf(stderr, "Warning: adding default scene before "
                "line no. %d\n", yylineno);
        newScene();
        s = g_presentation -> scenes;
    }
    return s -> direction;
}

void
setCursor(struct vec2d* x){
    struct scene *s = getLast(g_presentation -> scenes);
    if (s) {
        memcpy(&s -> cursor, x, sizeof(struct vec2d));
    } else {
        fprintf(stderr, "Error: no scene at line no %d\n", yylineno);
        abort();
    }
}

void
getCursor(struct vec2d* x){
    struct scene *s = getLast(g_presentation -> scenes);
    if (!s) {
        fprintf(stderr, "Warning: adding default scene before "
                "line no. %d\n", yylineno);
        newScene();
        s = g_presentation -> scenes;
    }
    memcpy(x, &s -> cursor, sizeof(struct vec2d));
}

void
getLoc(struct primitive* p, struct vec2d* v, uint8_t corner) {
    switch (corner) {
        case 0: (*v) = p -> c; break;

        case 1: (*v) = p -> n; break;
        case 4: (*v) = p -> e; break;
        case 8: (*v) = p -> w; break;
        case 2: (*v) = p -> s; break;

        case 5: (*v) = p -> ne; break;
        case 9: (*v) = p -> nw; break;
        case 6: (*v) = p -> se; break;
        case 10: (*v) = p -> sw; break;

        case 3: (*v) = p -> end; break;
        case 12: (*v) = p -> start; break;
        case 15:
            if (p -> t > 2 && p -> t < 8) {
                (*v) = p -> start;
            } else {
                (*v) = p -> c;
            }
            break;
    }
}

struct location*
getLastSegment(struct primitive *p){
    if (!p -> segments) {
        p -> segments = malloc(sizeof(struct location));
        p -> segments -> next = NULL;
        p -> segments -> x = p -> start.x;
        p -> segments -> y = p -> start.y;
    }
    return getLast(p -> segments);
}

struct textList*
addTextList(RsvgHandle *h, uint8_t positioning, struct textList *t){
    struct textList* t1 = getLast(t);

    if (!t1) {
        t1 = malloc(sizeof(struct textList));
        t = t1;
    } else {
        t1 -> next = malloc(sizeof(struct textList));
        t1 = t1 -> next;
    }
    
    t1 -> next = NULL;
    t1 -> h = h;
    t1 -> positioning = positioning;

    return t;
}
