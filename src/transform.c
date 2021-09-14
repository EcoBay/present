#include "transform.h"
#include "draw.h"
#include <string.h>

static cairo_surface_t *g_temp_surface = NULL;
static cairo_t *g_temp_context = NULL;
static uint32_t *g_temp_buf = NULL;
static size_t BUF_WIDTH, BUF_HEIGHT, BUF_STRIDE, BUF_SIZE;

static void
initTransform() {
    if (!g_temp_surface) {
        g_temp_surface = cairo_image_surface_create(
                CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
        g_temp_context = cairo_create(g_temp_surface);
        g_temp_buf = (uint32_t*) cairo_image_surface_get_data(
                g_temp_surface);

        cairo_translate(g_temp_context, WIDTH / 2, HEIGHT / 2);
        cairo_scale(g_temp_context, DPI, -DPI);

        BUF_STRIDE = cairo_image_surface_get_stride(g_temp_surface) >> 2;
        BUF_HEIGHT = cairo_image_surface_get_height(g_temp_surface);
        BUF_WIDTH = cairo_image_surface_get_width(g_temp_surface);
        BUF_SIZE = BUF_STRIDE * BUF_HEIGHT;
    }
}

static size_t
countPixels(uint32_t* buf, size_t count) {
    size_t ctr = 0;
    for (int i = 0; i < count; i++) {
        if (buf[i]) {
            ctr++;
        }
    }
    return ctr;
}

static struct pixel*
getPixels(uint32_t* buf, size_t count, int height, int width, int stride) {

    struct pixel *p;
    p = malloc(count * sizeof(struct pixel));
    int i = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int j = y * stride + x;
            if (buf[j]) {
                p[i].pos.x = x;
                p[i].pos.y = y;
                p[i].a = buf[j] & 0xff;
                p[i].r = buf[j] >> 8 & 0xff;
                p[i].g = buf[j] >> 16 & 0xff;
                p[i].b = buf[j] >> 24 & 0xff;
                i++;
            }
        }
    }

    return p;
}

static struct pixel*
populatePixel(struct pixel* p, size_t count, size_t desired) {
    if (count < desired) {
        p = realloc(p, desired * sizeof(struct pixel));
        for (int i = 0; i < desired - count; i++) {
            p[count + i] = p[i];
        }
    }
    return p;
}

static int
comparePointsX(const void *p, const void *q) {
    int i = ((struct pixel*) p) -> pos.x;
    int j = ((struct pixel*) q) -> pos.x;
    return i - j;
}

static int
comparePointsY(const void *p, const void *q) {
    int i = ((struct pixel*) p) -> pos.y;
    int j = ((struct pixel*) q) -> pos.y;
    return i - j;
}

static void
arrangePixel(struct pixel *p, size_t n, int a) {
    if (n > 1) {
        if (a) {
            qsort((void*) p, n, sizeof(struct pixel), comparePointsY);
        } else {
            qsort((void*) p, n, sizeof(struct pixel), comparePointsX);
        }
        arrangePixel(p, n/2 - 1, !a);
        arrangePixel(p + n/2, n/2, !a);
    }
}

void
prepareTransform(struct event *e, struct primitive *p) {
    initTransform();

    memset(g_temp_buf, 0, BUF_SIZE * sizeof(uint32_t));
    drawPrimitive(g_temp_context, p);
    size_t from_count = countPixels(g_temp_buf, BUF_SIZE);
    struct pixel *from;
    from = getPixels(g_temp_buf, from_count,
            BUF_HEIGHT, BUF_WIDTH, BUF_STRIDE);

    memset(g_temp_buf, 0, BUF_SIZE * sizeof(uint32_t));
    drawPrimitive(g_temp_context, e -> pr);
    size_t to_count = countPixels(g_temp_buf, BUF_SIZE);
    struct pixel *to;
    to = getPixels(g_temp_buf, to_count,
            BUF_HEIGHT, BUF_WIDTH, BUF_STRIDE);

    const size_t count = MAX(to_count, from_count);
    from = populatePixel(from, from_count, count);
    to = populatePixel(to, to_count, count);

    qsort(from, count, sizeof(struct pixel), comparePointsY);
    qsort(to, count, sizeof(struct pixel), comparePointsY);
    arrangePixel(from, count, 0);
    arrangePixel(to, count, 0);

    struct particles *par;
    par = malloc(sizeof(struct particles));
    par -> from = from;
    par -> to = to;
    par -> count = count;
    e -> par = par;
}

void
transformSetSource(cairo_t *cr, struct event *e, float p) {
    memset(g_temp_buf, 0, BUF_SIZE * sizeof(uint32_t));

#define LERP(T, P0, P1) ((T) * (P0) + (1.0 - (T)) * (P1))
    struct particles *par = e -> par;
    for (size_t i = 0; i < par -> count; i++) {
        size_t x = LERP(p, par -> to[i].pos.x, par -> from[i].pos.x);
        size_t y = LERP(p, par -> to[i].pos.y, par -> from[i].pos.y);
        uint32_t a = LERP(p, par -> to[i].a, par -> from[i].a);
        uint32_t r = LERP(p, par -> to[i].r, par -> from[i].r);
        uint32_t g = LERP(p, par -> to[i].g, par -> from[i].g);
        uint32_t b = LERP(p, par -> to[i].b, par -> from[i].b);
        uint32_t color = a | r << 8 | g << 16 | b << 24;
        g_temp_buf[y * BUF_STRIDE + x] = color;
    }
#undef LERP

    cairo_pattern_t *pat;
    pat = cairo_pattern_create_for_surface(g_temp_surface);
    cairo_matrix_t matrix;
    cairo_get_matrix(g_temp_context, &matrix);
    cairo_pattern_set_matrix(pat, &matrix);

    cairo_set_source(cr, pat);
    cairo_pattern_destroy(pat);
}

void
cleanTransform() {
    if (g_temp_surface) {
        cairo_destroy(g_temp_context);
        cairo_surface_destroy(g_temp_surface);
    }
}
