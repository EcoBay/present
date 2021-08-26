#include "tex.h"
#include "draw.h"
#include "object.h"
#include "present.h"
#include "symtable.h"
#include "present.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

const struct {
    char *sym;
    float val;
} builtIn[] = {
    {"boxwid", 0.75},
    {"boxht", 0.5},
    {"circlerad", 0.25},
    {"arcrad", 0.25},
    {"ellipsewid", 0.75},
    {"ellipseht", 0.5},
    {"linewid", 0.5},
    {"lineht", 0.5},
    {"movewid", 0.5},
    {"moveht", 0.5},
    {"textwid", 0},
    {"textht", 0},
    {"arrowwid", 0.05},
    {"arrowht", 0.1},
    {"dashwid", 0.1},
    {"arrowhead", 3.0},
    {"maxpswid", 24},
    {"maxpsht", 8.5},
    {"scale", 1},
    {"fillval", 0.3},
    {"ps", 20},
    {"vs", 30}
};

static void catchErr(int signo) {
    cleanTexDir();
    if (signo == SIGINT) {
        write(STDERR_FILENO, "Interrupted\n", 12);
    }
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    // struct sigaction sa;
    // sa.sa_handler = catchErr;

    // sigaction(SIGABRT, &sa, NULL);
    // sigaction(SIGINT, &sa,  NULL);

    char opt;
    char *out = strdup("out.mp4");
    while ((opt = getopt(argc, argv, "+:ilGO:")) != EOF) {
        switch (opt) {
            case 'l':
                // TODO: layout preprocessor
                break;
            case 'i':
                // TODO: Interactive mode
                break;
            case 'G':
                // TODO: Custom grap preprocessor
                break;
            case 'O':
                free(out);
                out = strdup(optarg);
                break;
            case '?':
                fprintf(stderr, "Error: unrecognized option '%c'\n",
                        optopt);
                exit(EXIT_FAILURE);
                break;
            case ':':
                fprintf(stderr, "Error: '%c' option requires "
                        "an argument\n", optopt);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (optind < argc) {
        if (!strcmp(argv[optind], "-")) {
            yyin = stdin;
        } else {
            if (!(yyin = fopen(argv[optind], "r"))) {
                fprintf(stderr, "Error: input file %s can not "
                        "be read\n", argv[optind]);
                exit(EXIT_FAILURE);
            }
        }
    } else {
        yyin = stdin;
    }

    if (optind + 1 < argc) {
        free(out);
        out = strdup(argv[optind + 1]);
    }

    if (!strcmp(out, "-")) {
        free(out);
        out = strdup("-f matroska -");
    }

    pushTable();
    int c = sizeof(builtIn) / sizeof(builtIn[0]);
    for (int i = 0; i < c; i++) {
        union T v = { .d = builtIn[i].val };
        setSym(builtIn[i].sym, SYM_DOUBLE, v);
    }

    initPresentation();
    pushTable();        // For macro and type checking during parsing and lexing
    while (yyparse());

    struct symTable *t;
    while (t = popTable()){
        freeTable(t);
    }

    renderPresentation(out);
    free(out);

    return 0;
}
