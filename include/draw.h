#ifndef DRAW_H
#define DRAW_H

#include "object.h"
#include <cairo.h>

extern int WIDTH;
extern int HEIGHT;
extern int DPI;

void renderPresentation(const char*);
void drawPrimitive(cairo_t*, struct primitive*);

#endif
