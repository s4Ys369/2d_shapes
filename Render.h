#ifndef RENDER_H
#define RENDER_H

#include <libdragon.h>
#include "Point.h"
#include "Shape.h"
#include "Utils.h"

class Render{
public:

    void draw_ellipse(float cx, float cy, float rx, float ry, int segments, std::vector<Point>& points);
    void draw_fan_curved(const std::vector<Point>& points);
    void draw_line(float x1, float y1, float x2, float y2, float thickness);
    void draw_bezier_curve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments, float thickness);
    void fill_between_beziers(const std::vector<Point>& curve1, const std::vector<Point>& curve2);
    void draw_filled_beziers(const std::vector<Point>& upper_curve, const std::vector<Point>& lower_curve, int segments);
    bool is_ear(const std::vector<Point>& polygon, int u, int v, int w, const std::vector<int>& V);
    void triangulate_polygon(const std::vector<Point>& polygon, std::vector<Point>& triangles);
    void draw_filled_bezier_shape(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments);
    void draw_fan_transform(const std::vector<Point>& point, const std::vector<float>& p2, int segments, float width);
    void fill_edge_to_ellipse(const std::vector<Point>& currentPoints, int segments, float scale);
};


#endif // RENDER_H