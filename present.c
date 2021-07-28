#include "draw.h"
#include "object.h"
#include "present.h"
#include "present.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int d = 3;
static float x = 0.0, y = 0.0;

int main(int argc, char **argv){
    initPresentation();
    while (yyparse());
    renderPresentation();
    return 0;
}
