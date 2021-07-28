#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <cairo.h>

extern struct presentation *g_presentation;

enum primitiveType {
    PRIM_BOX,
    PRIM_ELLIPSE,
    PRIM_CIRCLE,
    PRIM_ARC,
    PRIM_LINE,
    PRIM_ARROW,
    PRIM_SPLINE,
    PRIM_MOVE,
    PRIM_TEXT_LIST
};

enum easingFunction {
    EASE_LINEAR,
    EASE_IN_SINE,
    EASE_OUT_SINE,
    EASE_IN_OUT_SINE,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_IN_OUT_CUBIC
};

struct vec2d {
    float x, y;
};

struct location {
    struct location *next;
    float x, y;
};

struct color {
    uint8_t r, g, b, a;
};

struct textList {
    struct textList *next;
    uint8_t positioning;
    char *s;
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
    float dotted, dashed;
    float chop;
    uint8_t arrowStyle;
    uint8_t flags;
    struct color *fill;
    struct textList *txt;
};

union eventState {
    struct particle *pt;
    struct primitive *pr;
};

struct event {
    struct event *next;

    /* Event Type
     * 0 - Draw
     * 1 - Transform
     */
    uint8_t eventType;
    union eventState a;
    cairo_pattern_t *overlay;
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

    /* Cursor information */
    struct vec2d cursor;
    int direction;
};

struct presentation {
    struct scene *scenes;
};

void* getLast(void*);

void newScene();
void newKeyframe(float duration, enum easingFunction);
void newDrawEvent(struct primitive*);
struct primitive* newPrimitive(enum primitiveType);
void preparePrimitive(struct primitive*);

void initPresentation();

void setDirection(uint8_t);
uint8_t getDirection();
void setCursor(struct vec2d*);
void getCursor(struct vec2d*);

struct location* getLastSegment(struct primitive*);
#endif
