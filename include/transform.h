#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "object.h"
#include <cairo.h>

struct pixel {
    struct vec2d pos;
    uint8_t r, g, b, a;
};

struct particles {
    struct pixel* from;
    struct pixel* to;
    size_t count;
};

void prepareTransform(struct event*, struct primitive*);
void transformSetSource(cairo_t*, struct event*, float);
void cleanTransform();

#endif
