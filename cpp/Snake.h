#ifndef SNAKE_H
#define SNAKE_H

#include <libdragon.h>
#include "Chain.h"
#include "Utils.h"

extern int TC; // Snake's tri count


class Snake {
public:
    Chain spine;
    std::vector<float> bodyWidth;

    Snake(PVector origin);
    void resolve(float mouseX, float mouseY);

    float getBodyWidth(int i);
    float getPosX(int i, float angleOffset, float lengthOffset);
    float getPosY(int i, float angleOffset, float lengthOffset);
    
    void draw_ellipse(float cx, float cy, float rx, float ry);
    void get_ellipse_points(float cx, float cy, float rx, float ry, int segments, std::vector<PVector>& points);
    void draw_snake_shape();
};

#endif // SNAKE_H