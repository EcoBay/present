#include "draw.h"
#include "object.h"
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

const int WIDTH = 960;
const int HEIGHT = 720;
const int FRAMERATE = 24;
const int DPI = 96;
const char *OUTNAME = "test.mp4";

static inline void
drawRectangle(cairo_t *cr, struct primitive *p){
    cairo_rectangle(cr,
            p -> nw.x, p -> nw.y,
            p -> se.x - p -> nw.x, p -> se.y - p -> nw.y);

    cairo_set_line_width(cr, 1.0 / DPI);
    cairo_stroke(cr);
}

static inline void
drawEllipse(cairo_t *cr, struct primitive *p){
    cairo_matrix_t save_matrix;
    cairo_get_matrix(cr, &save_matrix);

    cairo_translate(cr, p -> c.x, p -> c.y);
    cairo_scale(cr, p -> wid / 2.0, p -> ht / 2.0);
    cairo_arc(cr,
            0, 0,
            1, 0, 2 * M_PI);

    cairo_set_matrix(cr, &save_matrix);

    cairo_set_line_width(cr, 1.0 / DPI);
    cairo_stroke(cr);
}

static inline void
drawCircle(cairo_t *cr, struct primitive *p){
    cairo_new_path(cr);
    cairo_arc(cr,
            p -> c.x, p -> c.y,
            p -> rad,
            0, 2 * M_PI);
    cairo_close_path(cr);

    cairo_set_line_width(cr, 1.0 / DPI);
    cairo_stroke(cr);
}

static inline void
renderDrawEvent(cairo_surface_t *surface, cairo_t *cr, struct event *e){
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    struct primitive *p = e -> a.pr;
    switch (p -> t) {
        case PRIM_BOX:
            drawRectangle(cr, p);
            break;
        case PRIM_ELLIPSE:
            drawEllipse(cr, p);
            break;
        case PRIM_CIRCLE:
            drawCircle(cr, p);
            break;
        case PRIM_ARC:
        case PRIM_LINE:
        case PRIM_ARROW:
        case PRIM_SPLINE:
        case PRIM_MOVE:
    }
};

static void
renderEvent(cairo_surface_t *surface, cairo_t *cr, struct event *e){
    if (e -> eventType == 0) {
        renderDrawEvent(surface, cr, e);
    } else {
    }
}

void
renderPresentation(){
    char *ffmpeg_cmd = malloc(256);

    snprintf(ffmpeg_cmd, 256,
            "ffmpeg -r %d -f rawvideo -pix_fmt bgra "
            "-s %dx%d -i - -threads 0 -preset fast "
            "-y -pix_fmt yuv420p -crf 21 %s",
            FRAMERATE, WIDTH, HEIGHT, OUTNAME);

    FILE *ffmpeg = popen(ffmpeg_cmd, "w");
    if (!ffmpeg) {
        fprintf(stderr, "Error: cannot open ffmpeg instance\n");
        exit(EXIT_FAILURE);
    }

    cairo_surface_t *surface = cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
    cairo_t *cr = cairo_create(surface);
    uint8_t *buf = cairo_image_surface_get_data(surface);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    cairo_translate(cr, WIDTH / 2, HEIGHT / 2);
    cairo_scale(cr, DPI, -DPI);

    for (struct scene *s = g_presentation -> scenes; s; s = s -> next) {
        for (struct keyframe *k = s -> keyframes; k; k = k -> next) {
            for (int i = 0; i < k -> duration * FRAMERATE; i++) {
                for (struct event *e = k -> events; e; e = e -> next) {
                    renderEvent(surface, cr, e);
                }
                fwrite(buf, WIDTH * HEIGHT * 4, 1, ffmpeg);
            }
        }
    }


    pclose(ffmpeg);
};

