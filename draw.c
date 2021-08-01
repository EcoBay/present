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
drawArrowhead(cairo_t *cr, struct primitive *p){
    struct vec2d l0 = p -> start;
    struct vec2d l1 = { p -> segments -> x, p -> segments -> y };

    if ( p -> arrowStyle & 2 ) {
        cairo_matrix_t save_matrix;
        cairo_get_matrix(cr, &save_matrix);

        float angle = atanf( ( l0.y - l1.y ) / ( l0.x - l1.x) );
        cairo_translate(cr, l0.x, l0.y);
        cairo_rotate(cr, angle);

        cairo_new_path(cr);
        cairo_move_to(cr, 1.0 / 10, -1.0 / 32);
        cairo_line_to(cr, 0, 0);
        cairo_line_to(cr, 1.0 / 10,  1.0 / 32);
        cairo_close_path(cr);

        cairo_fill(cr);

        cairo_set_matrix(cr, &save_matrix);
    }

    if ( p -> arrowStyle & 1 ) {
        for(struct location *l = p -> segments -> next; l; l = l -> next){
            l0 = l1;
            l1 = (struct vec2d) { l -> x, l -> y };
        }

        cairo_matrix_t save_matrix;
        cairo_get_matrix(cr, &save_matrix);

        float angle = atanf( ( l1.y - l0.y ) / ( l1.x - l0.x) );
        cairo_translate(cr, l1.x, l1.y);
        cairo_rotate(cr, angle);

        cairo_new_path(cr);
        cairo_move_to(cr, -1.0 / 10, -1.0 / 32);
        cairo_line_to(cr, 0, 0);
        cairo_line_to(cr, -1.0 / 10,  1.0 / 32);
        cairo_close_path(cr);

        cairo_fill(cr);

        cairo_set_matrix(cr, &save_matrix);
    }
}

static inline void
drawLine(cairo_t *cr, struct primitive *p){
    cairo_new_path(cr);
    cairo_move_to(cr, p -> start.x, p -> start.y);

    for (struct location *l = p -> segments; l; l = l -> next) {
        cairo_line_to(cr, l -> x, l -> y);
    }

    cairo_set_line_width(cr, 1.0 / DPI);
    cairo_stroke(cr);

    drawArrowhead(cr, p);
}

static inline void
drawSpline(cairo_t *cr, struct primitive *p){
}

static inline void
renderDrawEvent(cairo_surface_t *surface, cairo_t *cr, struct event *e){
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    struct primitive *p = e -> a.pr;
    switch (p -> t) {
        case PRIM_BOX:
            drawRectangle(cr, p); break;
        case PRIM_ELLIPSE:
            drawEllipse(cr, p); break;
        case PRIM_CIRCLE:
            drawCircle(cr, p); break;
        case PRIM_ARC:
            break;
        case PRIM_LINE:
        case PRIM_ARROW:
            drawLine(cr, p); break;
        case PRIM_SPLINE:
            drawSpline(cr, p); break;
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

