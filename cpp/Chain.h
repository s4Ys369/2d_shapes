#ifndef CHAIN_H
#define CHAIN_H

#include <libdragon.h>
#include <vector>
#include "PVector.h"

class Chain {
public:
    std::vector<PVector> joints;
    std::vector<float> angles;
    int linkSize; // Space between joints
    float angleConstraint; // Max angle diff between two adjacent joints, higher = loose, lower = rigid

    Chain(PVector origin, int jointCount, int linkSize);
    Chain(PVector origin, int jointCount, int linkSize, float angleConstraint);

    void resolve(PVector pos);
    void fabrikResolve(PVector pos, PVector anchor);

    void draw_line(float x1, float y1, float x2, float y2);
    void draw_ellipse(float cx, float cy, float rx, float ry);
    void display();
};

#endif // CHAIN_H