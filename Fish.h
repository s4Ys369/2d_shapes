#ifndef FISH_H
#define FISH_H

#include <libdragon.h>
#include <vector>
#include "Chain.h"
#include "PVector.h"

extern int triCount; // Fish's tri count

class Fish {
public:
    Chain spine;
    color_t bodyColor;
    color_t finColor;
    std::vector<float> bodyWidth;

    Fish(PVector origin);
    void resolve(float mouseX, float mouseY);
    float getBodyWidth(int i);
    void display();

private:
    float getPosX(int i, float angleOffset, float lengthOffset);
    float getPosY(int i, float angleOffset, float lengthOffset);
    void draw_ellipse(float cx, float cy, float rx, float ry);
    void get_ellipse_points(float cx, float cy, float rx, float ry, int segments, std::vector<PVector>& points);
    void draw_fin(float posX, float posY, float rotation, float width, float height);
    float get_tail_width(int i, float headToTail);
    void draw_curve(const std::vector<PVector>& points);
    void draw_line(float x1, float y1, float x2, float y2, float thickness);
    void draw_bezier_curve(const PVector& p0, const PVector& p1, const PVector& p2, const PVector& p3, int segments);
    void fill_between_beziers(const std::vector<PVector>& curve1, const std::vector<PVector>& curve2);
    void draw_filled_beziers(const PVector& p0, const PVector& p1, const PVector& p2, const PVector& p3, 
                               const PVector& q0, const PVector& q1, const PVector& q2, const PVector& q3, 
                               int segments);
    bool is_ear(const std::vector<PVector>& polygon, int u, int v, int w, const std::vector<int>& V);
    void triangulate_polygon(const std::vector<PVector>& polygon, std::vector<PVector>& triangles);
    void draw_filled_bezier_shape(const PVector& p0, const PVector& p1, const PVector& p2, const PVector& p3, int segments);
    void draw_tail(const std::vector<PVector>& j, const std::vector<float>& a, float headToTail);
    void draw_body();
};

#endif // FISH_H