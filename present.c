#include "present.h"
#include "present.tab.h"
#include <string.h>
#include <stdio.h>

static int d = 3;
static float x = 0.0, y = 0.0;

void draw(char *primitive, char *direction){
    if (!strcmp(primitive, "box")){
        printf("Box with start at %.2f, %.2f ", x, y);
        if(d < 2) y += 0.5;
        else x += 0.75;
        printf("to %.2f, %.2f\n", x, y);
        changeDirection(direction);
    } else if (!strcmp(primitive, "circle")){
        printf("Circle with start at %0.2f, %.2f ", x, y);
        if(d < 2) y += 0.5;
        else x += 0.5;
        printf("to %.2f, %.2f\n", x, y);
        changeDirection(direction);
    } else if (!strcmp(primitive, "ellipse")){
        printf("Ellipse with start at %.2f, %.2f ", x, y);
        if(d < 2) y += 0.5;
        else x += 0.75;
        printf("to %.2f, %.2f\n", x, y);
        changeDirection(direction);
    } else if (!strcmp(primitive, "arc")){
        printf("Arc\n");
        changeDirection(direction);
    } else if (
            !strcmp(primitive, "line") |
            !strcmp(primitive, "arrow") |
            !strcmp(primitive, "spline") 
    ){
        changeDirection(direction);
        printf("%s with start at %.2f, %.2f ", primitive, x, y);
        switch (d){
            case 0: y += 0.5; break;
            case 1: y -= 0.5; break;
            case 2: x += 0.5; break;
            case 3: x -= 0.5; break;
        }
        printf("to %.2f, %.2f\n", x, y);
    } else 
        fprintf(stderr, "Invalid primitive: %s\n", primitive);

}

void changeDirection(char *direction){
    if (!strcmp(direction, "up")) d = 0;
    else if (!strcmp(direction, "down")) d = 1;
    else if (!strcmp(direction, "left")) d = 2;
    else if (!strcmp(direction, "right")) d = 3;
    else if (!strcmp(direction, "same")) d = d;
    else fprintf(stderr, "Invalid direction: %s\n", direction);

    printf("Changing direction to %s\n", direction);
}

int main(int argc, char **argv){
    while (yyparse());
    return 0;
}
