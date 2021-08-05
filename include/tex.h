#ifndef TEX_H
#define TEX_H

#include <librsvg/rsvg.h>
#include <object.h>

char* createTex(char*, int);
int tex2SVG(char*);

RsvgHandle* getSVGHandler(char*);
void getSVGDim(RsvgHandle*, struct vec2d*);

void cleanTexDir();

#endif
