#ifndef RENDER_H
#define RENDER_H

#include <libdragon.h>
#include "point.h"
#include "shapes.h"



void set_render_color(color_t color);
void set_random_render_color();
color_t get_random_render_color();
void render_move_point(PointArray* points, size_t index, float dx, float dy);
void render_move_shape_points(PointArray* points, float dx, float dy);
void render_rotate_point(PointArray* points, size_t index, Point center, float angle);
void render_rotate_shape_points(PointArray* points, Point center, float angle);
void render_get_ellipse_points(PointArray* previousPoints, Point center, float rx, float ry, int segments);
void draw_triangle(float* v1, float* v2, float* v3);
void draw_indexed_triangles(float* vertices, int vertex_count, int* indices, int index_count);
void draw_rdp_fan(const PointArray* pa, const Point center);
void draw_fan(const PointArray* pa, const Point center);
void draw_strip(float* v1, float* v2, float* v3, float* v4);
void draw_strip_from_array(float* vertices, int vertexCount, float width);
void draw_circle(float cx, float cy, float rx, float ry, float angle, float lod);
void draw_line(float x1, float y1, float x2, float y2, float thickness);
void draw_quad(float x1, float y1, float x2, float y2, float angle, float thickness);
void draw_bezier_curve(const Point* p0, const Point* p1, const Point* p2, const Point* p3, int segments, float angle, float thickness);
void fill_between_beziers(const PointArray* curve1, const PointArray* curve2);
void draw_filled_beziers(const Point* p0, const Point* p1, const Point* p2, const Point* p3, 
                         const Point* q0, const Point* q1, const Point* q2, const Point* q3, 
                         int segments);
bool is_ear(const PointArray* polygon, int u, int v, int w, const int* V);
void triangulate_polygon(const PointArray* polygon, PointArray* triangles);
void draw_filled_bezier_shape(const Point* p0, const Point* p1, const Point* p2, const Point* p3, int segments);
void draw_fan_transform(const PointArray* fan, float angle, int segments, float rx, float ry);
void fill_edge_ellipse_to_line(PointArray* previousPoints, PointArray* currentPoints, int segments, float scale);



#endif // RENDER_H