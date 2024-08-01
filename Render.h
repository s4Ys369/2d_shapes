#ifndef RENDER_H
#define RENDER_H

#include <libdragon.h>
#include "Point.h"
#include "Shape.h"
#include "Utils.h"

class Render{
public:

    void set_fill_color(color_t color);
    void move_point(std::vector<Point>& points, std::vector<Point>::size_type index, float dx, float dy);
    void move_shape_points(std::vector<Point>& points, float dx, float dy);
    void rotate_point(std::vector<Point>& points, std::vector<Point>::size_type index, Point center, float angle);
    void rotate_shape_points(std::vector<Point>& points, Point center, float angle);
    std::vector<Point> get_ellipse_points(Point center, float rx, float ry, int segments);
    void draw_indexed_triangles(float* vertices, int vertex_count, int* indices, int index_count);
    void draw_fan(const std::vector<Point>& points);
    void draw_ellipse(float cx, float cy, float rx, float ry, float angle, float lod);
    void draw_line(float x1, float y1, float x2, float y2, float angle, float thickness);
    void draw_bezier_curve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments, float thickness);
    void fill_between_beziers(const std::vector<Point>& curve1, const std::vector<Point>& curve2);
    void draw_filled_beziers(const Point& p0, const Point& p1, const Point& p2, const Point& p3, 
                               const Point& q0, const Point& q1, const Point& q2, const Point& q3, 
                               int segments);
    bool is_ear(const std::vector<Point>& polygon, int u, int v, int w, const std::vector<int>& V);
    void triangulate_polygon(const std::vector<Point>& polygon, std::vector<Point>& triangles);
    void draw_filled_bezier_shape(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments);
    void draw_fan_transform(const std::vector<Point>& points, float angle, int segments, float rx, float ry);
    void fill_edge_ellipse_to_line(const std::vector<Point>& currentPoints, int segments, float scale);
};


#endif // RENDER_H