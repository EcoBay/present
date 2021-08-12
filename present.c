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
    {"arrowhead", 2.0},
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
    struct sigaction sa;
    sa.sa_handler = catchErr;

    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGINT, &sa,  NULL);

    pushTable();
    int c = sizeof(builtIn) / sizeof(builtIn[0]);
    for (int i = 0; i < c; i++) {
        union T v = { .d = builtIn[i].val };
        defineSym(builtIn[i].sym, SYM_DOUBLE, v);
    }

    initPresentation();
    while (yyparse());
    renderPresentation();
    return 0;
}
