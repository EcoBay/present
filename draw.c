#include "tex.h"
#include "draw.h"
#include "object.h"
#include "symtable.h"
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

int WIDTH = 960;
int HEIGHT = 720;
int DPI = 96;
const static int FRAMERATE = 24;
const char *OUTNAME = "test.mp4";

static void
drawRectangle(cairo_t *cr, struct primitive *p){
    cairo_rectangle(cr,
            p -> nw.x, p -> nw.y,
            p -> se.x - p -> nw.x, p -> se.y - p -> nw.y);

    cairo_set_line_width(cr, 1.0 / DPI);
    cairo_stroke(cr);
}

static void
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

static void
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

static void
drawArrowhead(cairo_t *cr, struct vec2d* o, float angle){
    cairo_matrix_t save_matrix;
    cairo_get_matrix(cr, &save_matrix);

    cairo_translate(cr, o -> x, o -> y);
    cairo_rotate(cr, angle);

    struct symbol *s;
    GET_FLOAT_SYM(s, "arrowht");
    float ht = -s -> val.d;
    GET_FLOAT_SYM(s, "arrowwid");
    float wid = s -> val.d / 2.0;

    cairo_new_path(cr);
    cairo_move_to(cr, ht, -wid);
    cairo_line_to(cr, 0, 0);
    cairo_line_to(cr, ht,  wid);
    cairo_close_path(cr);

    cairo_fill(cr);

    cairo_set_matrix(cr, &save_matrix);
}

static float
getRotation(float x, float y){
    float theta = atanf(y / x);
    if (x < 0) theta += M_PI;
    else if (y < 0) theta += 2 * M_PI;

    for(;theta < 0; theta += 2 * M_PI);
    for(;theta > 2 * M_PI; theta -= 2 * M_PI);
    return theta;
}

static void
drawLineArrowhead(cairo_t *cr, struct primitive *p){
    struct vec2d l0 = p -> start;
    struct vec2d l1 = { p -> segments -> x, p -> segments -> y };

    if ( p -> arrowStyle & 2 ) {
        float angle = getRotation(l0.x - l1.x, l0.y - l1.y);
        drawArrowhead(cr, &l0, angle);
    }

    if ( p -> arrowStyle & 1 ) {
        for(struct location *l = p -> segments -> next; l; l = l -> next){
            l0 = l1;
            l1 = (struct vec2d) { l -> x, l -> y };
        }
        float angle = getRotation(l1.x - l0.x, l1.y - l0.y);
        drawArrowhead(cr, &l1, angle);
    }
}

static void
drawLine(cairo_t *cr, struct primitive *p){
    cairo_new_path(cr);
    cairo_move_to(cr, p -> start.x, p -> start.y);

    for (struct location *l = p -> segments; l; l = l -> next) {
        cairo_line_to(cr, l -> x, l -> y);
    }

    cairo_set_line_width(cr, 1.0 / DPI);
    cairo_stroke(cr);

    drawLineArrowhead(cr, p);
}

static void
drawSpline(cairo_t *cr, struct primitive *p){
    cairo_new_path(cr);
    cairo_move_to(cr, p -> start.x, p -> start.y);
    cairo_line_to(cr,
            (p -> start.x + p -> segments -> x) / 2.0,
            (p -> start.y + p -> segments -> y) / 2.0);

    struct location *l = p -> segments;
    struct vec2d l0 = { p -> start.x, p -> start.y };

    while (l -> next) {
        cairo_curve_to(cr,
                (l0.x + 4.0 * l -> x) / 5.0,
                (l0.y + 4.0 * l -> y) / 5.0,
                (4.0 * l -> x + l -> next -> x) / 5.0,
                (4.0 * l -> y + l -> next -> y) / 5.0,
                (l -> x + l -> next -> x) / 2.0,
                (l -> y + l -> next -> y) / 2.0);
        l0 = (struct vec2d) {l -> x, l -> y};
        l = l -> next;
    }

    cairo_line_to(cr, l -> x, l -> y);
    cairo_set_line_width(cr, 1.0 / DPI);
    cairo_stroke(cr);

    drawLineArrowhead(cr, p);
}

static void
drawTextList(cairo_t *cr, struct textList *t){
    cairo_matrix_t save_matrix;
    cairo_get_matrix(cr, &save_matrix);

    while (t) {
        cairo_translate(cr, t -> nw.x, t -> nw.y);
        cairo_scale(cr, 1.0 / DPI, -1.0 / DPI);

        rsvg_handle_render_cairo(t -> h, cr);

        cairo_set_matrix(cr, &save_matrix);
        t = t -> next;
    }
}

static void
drawArc(cairo_t *cr, struct primitive *p){
    cairo_new_path(cr);

    float theta0 = getRotation(p -> start.x - p -> c.x, p -> start.y - p -> c.y);
    float theta1 = getRotation(p -> end.x - p -> c.x, p -> end.y - p -> c.y);
    float cw = 1;

    if (p -> flags & 2) {
        cairo_arc_negative(cr,
                p -> c.x, p -> c.y,
                p -> rad, theta0, theta1);
        cw = -1;
    } else {
        cairo_arc(cr,
                p -> c.x, p -> c.y,
                p -> rad, theta0, theta1);
    }

    cairo_set_line_width(cr, 1.0 / DPI);
    cairo_stroke(cr);

    if ( p -> arrowStyle & 1 ) {
        drawArrowhead(cr, &p -> start, theta0 - M_PI / 2.0 * cw);
    }
    if ( p -> arrowStyle & 2 ) {
        drawArrowhead(cr, &p -> end, theta1 + M_PI / 2.0 * cw);
    }
}

static void
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
            drawArc(cr, p); break;
        case PRIM_LINE:
        case PRIM_ARROW:
            drawLine(cr, p); break;
        case PRIM_SPLINE:
            drawSpline(cr, p); break;
    }
    drawTextList(cr, p -> txt);
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
            "-y -pix_fmt yuv420p -crf 21 %s "
            "-hide_banner -loglevel error " ,
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

    cairo_translate(cr, WIDTH / 2, HEIGHT / 2);
    cairo_scale(cr, DPI, -DPI);

    for (struct scene *s = g_presentation -> scenes; s; s = s -> next) {
        for (struct keyframe *k = s -> keyframes; k; k = k -> next) {
            for (int i = 0; i < k -> duration * FRAMERATE; i++) {
                cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
                cairo_paint(cr);
                for (struct event *e = k -> events; e; e = e -> next) {
                    renderEvent(surface, cr, e);
                }
                fwrite(buf, WIDTH * HEIGHT * 4, 1, ffmpeg);
            }
        }
    }

    pclose(ffmpeg);
    cleanTexDir();
};

