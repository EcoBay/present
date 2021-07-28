#include "draw.h"
#include "object.h"
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>

static void
renderEvent(cairo_t *cr, struct event *e){
}

void
renderPresentation(){
    const int WIDTH = 960;
    const int HEIGHT = 720;
    const int FRAMERATE = 24;
    const char *OUTNAME = "test.mp4";
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

    for (struct scene *s = g_presentation -> scenes; s; s = s -> next) {
        for (struct keyframe *k = s -> keyframes; k; k = k -> next) {
            for (int i = 0; i < k -> duration * FRAMERATE; i++) {
                for (struct event *e = k -> events; e; e = e -> next) {
                    renderEvent(cr, e);
                }
                fwrite(buf, WIDTH * HEIGHT * 4, 1, ffmpeg);
            }
        }
    }


    pclose(ffmpeg);
};

