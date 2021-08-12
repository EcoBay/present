#include "tex.h"
#include "symtable.h"
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

static char *TEX_DIR = NULL;

static void
createDir() {
    TEX_DIR = malloc(25);
    snprintf(TEX_DIR, 25, "/tmp/present-%010ld/", time(NULL));

    if (mkdir(TEX_DIR, 0777) == -1) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
}

static unsigned
hash(char *s, int pt){
    unsigned h = pt;
    char c;
    while (c = *s++) h = h*9 ^ c;
    return h;
}

char*
createTex(char *s, int pt){
    if (!TEX_DIR) createDir();

    char *id = malloc(12);
    snprintf(id, 12, "%010u", hash(s, pt));

    char texFile[45];
    snprintf(texFile, 45, "%s%s.tex", TEX_DIR, id);

    FILE *texFD = fopen(texFile, "w");

    fputs("\\documentclass[10pt]{standalone}\n", texFD);
    fputs("\\begin{document}\n", texFD);

    struct symbol *sym;
    GET_FLOAT_SYM(sym, "maxpswid");
    int maxWid = (int) sym -> val.d;
    int wid = MIN((int) (pt * 1.2), maxWid);


    fprintf(texFD, "{\\fontsize{%d}{%d} \\selectfont %s}\n",
            pt, wid, s);

    fputs("\\end{document}\n", texFD);
    fclose(texFD);

    return id;
}

int tex2SVG(char *id){
    char texFile[45];
    snprintf(texFile, 45, "%s%s.tex", TEX_DIR, id);
    char pdfFile[45];
    snprintf(pdfFile, 45, "%s%s.pdf", TEX_DIR, id);
    char svgFile[45];
    snprintf(svgFile, 45, "%s%s.svg", TEX_DIR, id);
    
    char latexCMD[256];
    snprintf(latexCMD, 256,
            "pdflatex -halt-on-error -output-directory %s %s > /dev/null",
            TEX_DIR, texFile);

    char svgCMD[256];
    snprintf(svgCMD, 256, "pdf2svg %s %s", pdfFile, svgFile);

    int latexRet, svgRet;
    if (latexRet = system(latexCMD)){
        return latexRet;
    }

    if (svgRet = system(svgCMD)) {
        return svgRet;
    }

    return 0;
}

RsvgHandle* getSVGHandler(char *id){
    char svgFile[45];
    snprintf(svgFile, 45, "%s%s.svg", TEX_DIR, id);

    RsvgHandle *h;
    h = rsvg_handle_new_from_file(svgFile, NULL);

    if (!h) {
        fprintf(stderr, "Error: cannot open svg handler\n");
        abort();
    }

    return h;
}

void getSVGDim(RsvgHandle* h, struct vec2d* dim){
    RsvgDimensionData svgDim;
    rsvg_handle_get_dimensions(h, &svgDim);

    dim -> x = svgDim.width;
    dim -> y = svgDim.height;
}

void cleanTexDir(){
    if (TEX_DIR) {
        char rmCmd[256];
        snprintf(rmCmd, 256, "rm -rf %s", TEX_DIR);
        system(rmCmd);
    }
}
