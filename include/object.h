#ifndef OBJECT_H
#define OBJECT_H

#include <librsvg/rsvg.h>
#include <stdint.h>
#include <cairo.h>

extern struct presentation *g_presentation;
extern struct primitive *g_parent;

extern int g_sceneCtr;
extern struct symbol *g_sceneCtrSym;

#define NUM_PRIM_TYPE 10
enum primitiveType {
    PRIM_BOX,
    PRIM_ELLIPSE,
    PRIM_CIRCLE,
    PRIM_ARC,
    PRIM_LINE,
    PRIM_ARROW,
    PRIM_SPLINE,
    PRIM_MOVE,
    PRIM_TEXT_LIST,
    PRIM_BLOCK,
};

enum easingFunction {
    EASE_LINEAR,
    EASE_SINE,
    EASE_IN_SINE,
    EASE_OUT_SINE,
    EASE_QUAD,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_CUBIC,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC
};

struct vec2d {
    float x, y;
};

struct location {
    struct location *next;
    float x, y;
};

struct color {
    uint8_t a, b, g, r;
};

struct textList {
    struct textList *next;
    uint8_t positioning;
    RsvgHandle* h;
    struct vec2d nw;
};

struct primitive {
    enum primitiveType t;

    uint8_t direction;
    struct vec2d start;
    struct vec2d end;

    /* Locations */
    struct vec2d nw, n, ne;
    struct vec2d w,  c, e;
    struct vec2d sw, s, se;

    float expr;
    float ht, wid;
    float rad;
    struct location *segments;
    struct vec2d *at;
    uint8_t with;
    float spacing;
    float chop1;
    float chop2;
    uint8_t arrowStyle;
    uint32_t flags;
    /*   1 - hasSegment
     *   2 - cw
     *   4 - dashed
     *   8 - dotted
     *  16 - invis
     *  32 - filled
     *  64 - hasRadius
     * 128 - hasChop
     */
    struct color *fill;
    struct textList *txt;

    /* block primitive */
    struct symTable *tb;
    struct event *child;

    /* draw states */
    float ps;
    float arrowht;
    float arrowwid;
    uint8_t arrowhead;
};

struct event {
    struct event *next;

    /* Event Type
     * 0 - Draw
     * 1 - Transform
     */
    uint8_t eventType;
    struct primitive *pr;

    struct particle *par;
    cairo_pattern_t *pat;
};

struct keyframe {
    struct keyframe *next;
    struct event *events;
    float duration;                 // in seconds
    enum easingFunction easingFunc;
};

struct scene {
    struct scene *next;
    struct keyframe *keyframes;
    char *name;

    /* Cursor information */
    struct vec2d cursor;
    int direction;
};

struct presentation {
    struct scene *scenes;
};

void* getLast(void*);

void newScene(char*);
void newKeyframe(float duration, enum easingFunction);
struct event*  newDrawEvent(struct primitive*);
struct primitive* newPrimitive(enum primitiveType);
void preparePrimitive(struct primitive*);

void initPresentation();

void setDirection(uint8_t);
uint8_t getDirection();
void setCursor(struct vec2d*);
void getCursor(struct vec2d*);
void getLoc(struct primitive*, struct vec2d*, uint8_t corner);

struct location* getLastSegment(struct primitive*);
struct textList* addTextList(RsvgHandle*, uint8_t, struct textList*);
#endif
