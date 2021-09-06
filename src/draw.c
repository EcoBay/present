#include "tex.h"
#include "draw.h"
#include "object.h"
#include "symtable.h"
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

int WIDTH = 960;
int HEIGHT = 720;
int DPI = 96;
const static int FRAMERATE = 24;

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
}

static void
drawCircle(cairo_t *cr, struct primitive *p){
    float rad = (p -> flags & 64) ? p -> rad : p -> expr;

    cairo_new_path(cr);
    cairo_arc(cr,
            p -> c.x, p -> c.y, rad,
            0, 2 * M_PI);
    cairo_close_path(cr);
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
    if (p -> flags & 16) return;
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
    drawLineArrowhead(cr, p);
    cairo_new_path(cr);
    cairo_move_to(cr, p -> start.x, p -> start.y);

    for (struct location *l = p -> segments; l; l = l -> next) {
        cairo_line_to(cr, l -> x, l -> y);
    }

}

static void
drawSpline(cairo_t *cr, struct primitive *p){
    drawLineArrowhead(cr, p);
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
    float rad = (p -> flags & 64) ? p -> rad : p -> expr;
    float theta0 = getRotation(p -> start.x - p -> c.x, p -> start.y - p -> c.y);
    float theta1 = getRotation(p -> end.x - p -> c.x, p -> end.y - p -> c.y);
    float cw = 1;

    if (!(p -> flags & 16)) {
        if ( p -> arrowStyle & 2 ) {
            float ps = (p -> arrowhead & 2) ? 0.0f : p -> ps;
            drawArrowhead(cr, &p -> start, theta0 - M_PI / 2.0 * cw, p -> arrowht, p -> arrowwid, ps);
        }
        if ( p -> arrowStyle & 1 ) {
            float ps = (p -> arrowhead & 1) ? 0.0f : p -> ps;
            drawArrowhead(cr, &p -> end, theta1 + M_PI / 2.0 * cw, p -> arrowht, p -> arrowwid, ps);
        }
    }

    cairo_new_path(cr);
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
}

static struct vec2d*
getExtremes(struct primitive *p) {
    float x0 = p -> c.x, y0 = p -> c.y;
    float x1 = p -> c.x, y1 = p -> c.y;

#define GET_BOUND(L)    \
    x0 = MIN(x0, L.x);  \
    y0 = MIN(y0, L.y);  \
    x1 = MAX(x1, L.x);  \
    y1 = MAX(y1, L.y);

    GET_BOUND(p -> n);
    GET_BOUND(p -> e);
    GET_BOUND(p -> w);
    GET_BOUND(p -> s);

    GET_BOUND(p -> ne);
    GET_BOUND(p -> nw);
    GET_BOUND(p -> se);
    GET_BOUND(p -> sw);

    for (struct textList *t = p -> txt; t; t = t -> next) {
        GET_BOUND(t -> nw);
        GET_BOUND(t -> se);
    }
#undef GET_BOUND

    x0 -= 0.1;
    y0 -= 0.1;
    x1 += 0.1;
    y1 += 0.1;

    struct vec2d *r = malloc(3 * sizeof(struct vec2d));
    r[0].x = x0;
    r[0].y = y0;
    r[1].x = (x0 + x1) / 2.0;
    r[1].y = (y0 + y1) / 2.0;
    r[2].x = x1;
    r[2].y = y1;

    return r;
}

static void
prepareMask(cairo_t *cr, struct primitive *p, float f) {
    struct vec2d *e = getExtremes(p);

    switch (p -> direction) {
        case 0:
            cairo_rectangle(cr, e[0].x, e[0].y,
                    e[2].x - e[0].x, f * (e[2].y - e[0].y));
            break;
        case 1:
            cairo_rectangle(cr, e[0].x, e[0].y,
                    f * (e[2].x - e[0].x), e[2].y - e[0].y);
            break;
        case 2:
            cairo_rectangle(cr, e[2].x, e[2].y,
                    e[0].x - e[2].x, f * (e[0].y - e[2].y));
            break;
        case 3:
            cairo_rectangle(cr, e[2].x, e[2].y,
                    f * (e[0].x - e[2].x), e[0].y - e[2].y);
            break;
    }
}

static void
renderEvent(cairo_surface_t *surface, cairo_t *cr, struct event *e, float p){
    if (e -> eventType == 0) {
        cairo_set_source(cr, e -> pat);
        prepareMask(cr, e -> pr, p);
        cairo_fill(cr);
    } else {
    }
}

static cairo_pattern_t*
prepareDrawEvent(cairo_surface_t *surface, cairo_t *cr, struct event *e){
    struct primitive *p = e -> pr;
    cairo_push_group(cr);

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
            {
                struct event *e = p -> child;
                for (; e; e = e -> next) {
                    e -> pat = prepareDrawEvent(surface, cr, e);
                    renderEvent(surface, cr, e, 1.0);
                }
            }
            break;
        case PRIM_MOVE:
        case PRIM_TEXT_LIST:
            break;
    }

    if (p -> flags & 32) {
        cairo_set_source_rgba(cr,
            p -> fill -> r / 255.0, p -> fill -> g / 255.0,
            p -> fill -> b / 255.0, p -> fill -> a / 255.0);
        cairo_fill_preserve(cr);
    }
    if (!(p -> flags & 16)) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, p -> ps / 20.0 / DPI);
        if (p -> flags & 4) {
            dashed(cr, p -> spacing);
        } else if (p -> flags & 8) {
            dotted(cr, p -> spacing, p -> ps);
        } else {
            cairo_set_dash(cr, NULL, 0, 0);
        }
        cairo_stroke(cr);
    } else {
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_stroke(cr);
    }

    drawTextList(cr, p -> txt);
    return cairo_pop_group(cr);
}

static void
prepareEvent(cairo_surface_t *surface, cairo_t *cr, struct event *e) {
    if (e -> eventType == 0) {
        e -> pat = prepareDrawEvent(surface, cr, e);
    } else {
    }
}

static float
ease(float i, float t, enum easingFunction easingFunc) {
    float x = i / t;
    switch (easingFunc) {
        case EASE_STILL:
            x = 1.0;
            break;
        case EASE_LINEAR:
            break;

        case EASE_SINE:
            x = -(cosf(M_PI * x) - 1.0) / 2.0;
            break;
        case EASE_IN_SINE:
            x = 1.0 - cosf((x * M_PI) / 2.0);
            break;
        case EASE_OUT_SINE:
            x = sinf((x * M_PI) / 2.0);
            break;

#define SQ(a) ((a) * (a))
        case EASE_QUAD:
            x = x < 0.5 ? 2.0 * SQ(x) : 1.0 - SQ(-2.0 * x + 2.0) / 2.0;
            break;
        case EASE_IN_QUAD:
            x = SQ(x);
            break;
        case EASE_OUT_QUAD:
            x = 1.0 - SQ(1.0 - x);
            break;
#undef SQ

#define CB(a) ((a) * (a) * (a))
        case EASE_CUBIC:
            x = x < 0.5 ? 4.0 * CB(x) : 1.0 - CB(-2.0 * x + 2.0) / 2.0;
            break;
        case EASE_IN_CUBIC:
            x = CB(x);
            break;
        case EASE_OUT_CUBIC:
            x = 1.0 - CB(1.0 - x);
            break;
#undef CB
    }
    return x;
}

void
renderPresentation(const char* filename){
    char *ffmpeg_cmd = malloc(256);

    snprintf(ffmpeg_cmd, 256,
            "ffmpeg -r %d -f rawvideo -pix_fmt bgra "
            "-s %dx%d -i - -threads 0 -preset ultrafast "
            "-y -pix_fmt yuv420p -crf 23 %s "
            "-hide_banner -loglevel error " ,
            FRAMERATE, WIDTH, HEIGHT, filename);

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
            for (struct event *e = k -> events; e; e = e -> next) {
                prepareEvent(surface, cr, e);
            }

            cairo_surface_flush(surface);
            memcpy(tempBuf, buf, buffSize);
            const int numFrame = k -> duration * FRAMERATE;
            for (int i = 0; i < numFrame; i++) {
                if (k -> events) {
                    memcpy(buf, tempBuf, buffSize);
                    cairo_surface_mark_dirty(surface);
                }
                for (struct event *e = k -> events; e; e = e -> next) {
                    float p = ease(i, numFrame, k -> easingFunc);
                    renderEvent(surface, cr, e, p);
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
