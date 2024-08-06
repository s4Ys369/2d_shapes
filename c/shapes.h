#ifndef SHAPE_H
#define SHAPE_H

#include <libdragon.h>
#include "point.h"
#include "render.h"
#include "utils.h"

typedef struct {
    PointArray* currPoints;
    Point center;
    float scaleX;
    float scaleY;
    int segments;
    float lod;
    color_t fillColor;
} Shape;

// Initialization functions
void shape_init(Shape* shape);
void fan_init(Shape* fan, Point origin, float scale, int segments, color_t fillColor);
void fan2_init(Shape* fan, Point origin, float scaleX, float scaleY, int segments, color_t fillColor);
void circle_init(Shape* circle, Point origin, float scale, float lod, color_t fillColor);
void strip_init(Shape* strip, Point origin, float scaleX, float scaleY, float thickness, int segments, color_t fillColor);

// Common functions for the Shape interface
void set_points(Shape* shape, PointArray* points);
PointArray* get_points(Shape* shape);
void set_thickness(Shape* shape, float thickness);
float get_thickness(const Shape* shape);
void set_scaleX(Shape* shape, float scaleX);
float get_scaleX(const Shape* shape);
void set_scaleY(Shape* shape, float scaleY);
float get_scaleY(const Shape* shape);
void set_center(Shape* shape, Point center);
Point get_center(const Shape* shape);
void set_segments(Shape* shape, int segments);
int get_segments(const Shape* shape);
void set_lod(Shape* shape, float lod);
float get_lod(const Shape* shape);
void set_fill_color(Shape* shape, color_t fillColor);
color_t get_fill_color(const Shape* shape);
void resolve(Shape* shape, float stickX, float stickY);
void destroy(Shape* shape);

#endif // SHAPE_H
