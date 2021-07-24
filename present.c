#include "present.h"
#include "present.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int d = 3;
static float x = 0.0, y = 0.0;

void draw(uint8_t primitive, int direction){
    char *a;
    switch (primitive) {
        case 0:
            printf("Box with start at %.2f, %.2f ", x, y);
            if(d < 2) y += 0.5;
            else x += 0.75;
            printf("to %.2f, %.2f\n", x, y);
            changeDirection(direction);
            break;
        case 1:
            printf("Circle with start at %0.2f, %.2f ", x, y);
            if(d < 2) y += 0.5;
            else x += 0.5;
            printf("to %.2f, %.2f\n", x, y);
            changeDirection(direction);
            break;
        case 2:
            printf("Ellipse with start at %.2f, %.2f ", x, y);
            if(d < 2) y += 0.5;
            else x += 0.75;
            printf("to %.2f, %.2f\n", x, y);
            changeDirection(direction);
            break;
        case 3:
            printf("Arc\n");
            changeDirection(direction);
            break;
        case 4:
            a = strdup("Line");
            goto drawLine;
        case 5:
            a = strdup("Arrow");
            goto drawLine;
        case 6:
            a = strdup("Spline");
            goto drawLine;
        drawLine:
            printf("%s with start at %.2f, %.2f ", a, x, y);
            switch (d){
                case 0: y += 0.5; break;
                case 1: y -= 0.5; break;
                case 2: x += 0.5; break;
                case 3: x -= 0.5; break;
            }
            printf("to %.2f, %.2f\n", x, y);
            free(a);
            break;
    }

}

void changeDirection(int direction){
    if (d >= 0) d = direction;
}

int main(int argc, char **argv){
    while (yyparse());
    return 0;
}
