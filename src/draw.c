#include "tex.h"
#include "draw.h"
#include "object.h"
#include "symtable.h"
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define SET_LINE_STYLE(CR, P) if (P -> flags & 4) { \
        dashed(CR, P -> spacing);                   \
    } else if (P -> flags & 8) {                    \
        dotted(CR, P -> spacing, P -> ps);          \
    } else {                                        \
        cairo_set_dash(CR, NULL, 0, 0);             \
    }
#define FILL_AND_STROKE(CR, P) if (P -> flags & 32) {           \
        cairo_set_source_rgba(CR,                               \
            P -> fill -> r / 255.0, P -> fill -> g / 255.0,     \
            P -> fill -> b / 255.0, P -> fill -> a / 255.0);    \
        cairo_fill_preserve(CR);                                \
    }                                                           \
    if (!(P -> flags & 16)) {                                   \
        cairo_set_source_rgb(CR,0, 0, 0);                       \
        cairo_set_line_width(CR, P -> ps / 20.0 / DPI);         \
        SET_LINE_STYLE(CR, P);                                  \
        cairo_stroke(CR);                                       \
    }

int WIDTH = 960;
int HEIGHT = 720;
int DPI = 96;
const static int FRAMERATE = 24;
const char *OUTNAME = "test.mp4";

static void
dotted(cairo_t *cr, float spacing, float ps) {
    double pat[] = {ps / 20.0 / DPI, spacing};
    cairo_set_dash(cr, pat, 2, 0);
}

static void
dashed(cairo_t *cr, float spacing) {
    double pat[] = {spacing, spacing * 0.5};
    cairo_set_dash(cr, pat, 2, 0);
}

static void
drawRectangle(cairo_t *cr, struct primitive *p){
    cairo_rectangle(cr,
            p -> nw.x, p -> nw.y,
            p -> se.x - p -> nw.x, p -> se.y - p -> nw.y);

    FILL_AND_STROKE(cr, p);
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
    FILL_AND_STROKE(cr, p);
}

static void
drawCircle(cairo_t *cr, struct primitive *p){
    float rad = (p -> flags & 64) ? p -> rad : p -> expr;

    cairo_new_path(cr);
    cairo_arc(cr,
            p -> c.x, p -> c.y, rad,
            0, 2 * M_PI);
    cairo_close_path(cr);

    FILL_AND_STROKE(cr, p);
}

static void
drawArrowhead(cairo_t *cr, struct vec2d* o, float angle, float arrowht, float arrowwid, float ps){
    cairo_matrix_t save_matrix;
    cairo_get_matrix(cr, &save_matrix);

    cairo_translate(cr, o -> x, o -> y);
    cairo_rotate(cr, angle);

    float ht = -arrowht;
    float wid = arrowwid / 2.0;

    cairo_new_path(cr);
    cairo_move_to(cr, ht, -wid);
    cairo_line_to(cr, 0, 0);
    cairo_line_to(cr, ht,  wid);

    if (ps == 0.0f) {
        cairo_close_path(cr);
        cairo_fill(cr);
    } else {
        cairo_set_line_width(cr, ps / 20.0 / DPI);
        cairo_set_dash(cr, NULL, 0, 0);
        cairo_stroke(cr);
    }

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
        float ps = (p -> arrowhead & 2) ? 0.0f : p -> ps;
        float angle = getRotation(l0.x - l1.x, l0.y - l1.y);
        drawArrowhead(cr, &l0, angle, p -> arrowht, p -> arrowwid, ps);
    }

    if ( p -> arrowStyle & 1 ) {
        for(struct location *l = p -> segments -> next; l; l = l -> next){
            l0 = l1;
            l1 = (struct vec2d) { l -> x, l -> y };
        }
        float ps = (p -> arrowhead & 1) ? 0.0f : p -> ps;
        float angle = getRotation(l1.x - l0.x, l1.y - l0.y);
        drawArrowhead(cr, &l1, angle, p -> arrowht, p -> arrowwid, ps);
    }
}

static void
drawLine(cairo_t *cr, struct primitive *p){
    cairo_new_path(cr);
    cairo_move_to(cr, p -> start.x, p -> start.y);

    for (struct location *l = p -> segments; l; l = l -> next) {
        cairo_line_to(cr, l -> x, l -> y);
    }

    cairo_set_line_width(cr, p -> ps / 20.0 / DPI);
    SET_LINE_STYLE(cr, p);
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
    cairo_set_line_width(cr, p -> ps / 20.0 / DPI);
    SET_LINE_STYLE(cr, p);
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

    float rad = (p -> flags & 64) ? p -> rad : p -> expr;
    float theta0 = getRotation(p -> start.x - p -> c.x, p -> start.y - p -> c.y);
    float theta1 = getRotation(p -> end.x - p -> c.x, p -> end.y - p -> c.y);
    float cw = 1;

    if (p -> flags & 2) {
        cairo_arc_negative(cr,
                p -> c.x, p -> c.y,
                rad, theta0, theta1);
        cw = -1;
    } else {
        cairo_arc(cr,
                p -> c.x, p -> c.y,
                rad, theta0, theta1);
    }

    cairo_set_line_width(cr, p -> ps / 20.0 / DPI);
    SET_LINE_STYLE(cr, p);
    cairo_stroke(cr);

    if ( p -> arrowStyle & 2 ) {
        float ps = (p -> arrowhead & 2) ? 0.0f : p -> ps;
        drawArrowhead(cr, &p -> start, theta0 - M_PI / 2.0 * cw, p -> arrowht, p -> arrowwid, ps);
    }
    if ( p -> arrowStyle & 1 ) {
        float ps = (p -> arrowhead & 1) ? 0.0f : p -> ps;
        drawArrowhead(cr, &p -> end, theta1 + M_PI / 2.0 * cw, p -> arrowht, p -> arrowwid, ps);
    }
}

static void
renderDrawEvent(cairo_surface_t *surface, cairo_t *cr, struct event *e){
    struct primitive *p = e -> a.pr;
    if (p -> flags & 16) return;

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
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
        case PRIM_BLOCK:
            struct event *e = p -> child;
            for (; e; e = e -> next) {
                renderDrawEvent(surface, cr, e);
            }
            break;
    }
    drawTextList(cr, p -> txt);
}

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
            "-s %dx%d -i - -threads 0 -preset ultrafast "
            "-y -pix_fmt yuv420p -crf 23 %s "
            "-hide_banner -loglevel error " ,
            FRAMERATE, WIDTH, HEIGHT, OUTNAME);

    FILE *ffmpeg = popen(ffmpeg_cmd, "w");
    if (!ffmpeg) {
        fprintf(stderr, "Error: cannot open ffmpeg instance\n");
        abort();
    }
    free(ffmpeg_cmd);

    cairo_surface_t *surface = cairo_image_surface_create(
            CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
    cairo_t *cr = cairo_create(surface);
    uint8_t *buf = cairo_image_surface_get_data(surface);

    size_t buffSize = cairo_image_surface_get_stride(surface) *
        cairo_image_surface_get_height(surface);
    uint8_t *tempBuf = malloc(buffSize);

    cairo_translate(cr, WIDTH / 2, HEIGHT / 2);
    cairo_scale(cr, DPI, -DPI);

    for (struct scene *s = g_presentation -> scenes; s; s = s -> next) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_paint(cr);
        for (struct keyframe *k = s -> keyframes; k; k = k -> next) {
            cairo_surface_flush(surface);
            memcpy(tempBuf, buf, buffSize);
            for (int i = 0; i < k -> duration * FRAMERATE; i++) {
                if (k -> events) {
                    memcpy(buf, tempBuf, buffSize);
                    cairo_surface_mark_dirty(surface);
                }
                for (struct event *e = k -> events; e; e = e -> next) {
                    renderEvent(surface, cr, e);
                }
                fwrite(buf, WIDTH * HEIGHT * 4, 1, ffmpeg);
            }
        }
    }

    free(tempBuf);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    pclose(ffmpeg);
    cleanTexDir();
};
