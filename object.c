#include "tex.h"
#include "object.h"
#include "present.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct presentation *g_presentation;

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
    }
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
    }

    struct scene *s = getLast(g_presentation -> scenes);
    if (!s -> keyframes) s -> keyframes = k;
    else {
        struct keyframe *t = s -> keyframes;
        t = getLast(t);
        t -> next = k;
    }
}

void
newDrawEvent(struct primitive *p){
    struct scene *s = g_presentation -> scenes;
    s = getLast(s);

    if (!s -> keyframes) {
        fprintf(stderr, "Warning: adding default keyframe before "
                "line no. %d\n", yylineno - 1);
        newKeyframe(1.0, EASE_LINEAR);
    }

    struct keyframe *k = getLast(s -> keyframes);

    struct event *e = malloc(sizeof(struct event));
    e -> next = NULL;
    e -> eventType = 0;
    e -> a.pr = p;

    if (!k -> events) {
        k -> events = e;
    } else {
        struct event *t = k -> events;
        t = getLast(t);
        t -> next = e;
    }

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
    p -> with = 0;
    p -> dotted = 0;
    p -> dashed = 0;
    p -> chop = 0;
    p -> arrowStyle = 0;
    p -> flags = 0;
    p -> fill = NULL;
    p -> txt = NULL;

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
    if(p -> t == PRIM_CIRCLE) wid = ht = p -> rad;

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

    // TODO: Change to lookup variable when symbol table is implemented
    float ht = 20 / 60.0;
    float curY = o -> y + ht * (c-1) / 2;

    if (ps && count) {
        *ps = malloc(2 * c * sizeof(struct vec2d));
        *count = 2 * c;
    }

    for (int i = 0; i < c; i++) {
        struct vec2d dim;
        getSVGDim(t -> h, &dim);

        dim.x /= 192.0;
        dim.y /= 192.0;

        t -> nw.y = curY + dim.y / 2;
        t -> nw.x = o -> x - dim.x / 2;

        if (ps && count) {
            (*ps)[i * 2] = t -> nw;
            (*ps)[i * 2 + 1].x = t -> nw.x + dim.x;
            (*ps)[i * 2 + 1].y = t -> nw.y - dim.y;
        }

        curY -= ht;
        t = t -> next;
    }
}


void
preparePrimitive(struct primitive *p){
    struct location *l;
    struct vec2d *ps;
    uint8_t count;

    switch (p -> t) {
        case PRIM_BOX:
        case PRIM_ELLIPSE:
        case PRIM_CIRCLE:
            float wid = (p -> t == PRIM_CIRCLE) ?
                p -> rad * 2 : p -> wid;
            float ht  = (p -> t == PRIM_CIRCLE) ?
                p -> rad * 2 : p -> ht;

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
                    case 3: l -> y -= e; break;
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
                        p -> start.y + p -> rad * invX * 2,
                    };
                    ps[2] = (struct vec2d) {
                        p -> start.x - p -> rad * invX,
                        p -> start.y + p -> rad * invY,
                    };
                    ps[3] = (struct vec2d) {
                        p -> start.x + p -> rad * invX,
                        p -> start.y + p -> rad * invY,
                    };
                    break;
                case 1:
                case 3:
                    ps[0] = (struct vec2d) {
                        p -> start.x,
                        p -> start.y
                    };
                    ps[1] = (struct vec2d) {
                        p -> start.x + p -> rad * invX * 2,
                        p -> start.y,
                    };
                    ps[2] = (struct vec2d) {
                        p -> start.x + p -> rad * invX,
                        p -> start.y - p -> rad * invY,
                    };
                    ps[3] = (struct vec2d) {
                        p -> start.x + p -> rad * invX,
                        p -> start.y + p -> rad * invY,
                    };
                    break;
            }

            p -> end = (p -> flags & 2) ? ps[2] : ps[3];
            setDirection(dir);
            break;
        case PRIM_TEXT_LIST:
            prepareTextList(p -> txt, &p -> start, &ps, &count);
            p -> end = p -> start;
            break;
    }

    updateBoundingBox(p, ps, count);
    chopBoundingBox(p);
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
        exit(EXIT_FAILURE);
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
