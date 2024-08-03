#ifndef SHAPE_H
#define SHAPE_H

#include <libdragon.h>
#include "point.h"
#include "Render.h"
#include "Utils.h"

typedef struct {
    Point* currPoints;
    size_t currPointsCount;
    Point* previousPoints;
    size_t previousPointsCount;
    Point center;
    float scaleX;
    float scaleY;
    int segments;
    float lod;
    color_t fillColor;
} Shape;

typedef struct {
    Point* currPoints;
    size_t currPointsCount;
    Point* previousPoints;
    size_t previousPointsCount;
    Point center;
    float scaleX;
    float scaleY;
    int segments;
    color_t fillColor;
} Fan;

typedef struct {
    Point* currPoints;
    size_t currPointsCount;
    Point* previousPoints;
    size_t previousPointsCount;
    Point center;
    float scale;
    float lod;
    color_t fillColor;
} Circle;

typedef struct {
    Point* currPoints;
    size_t currPointsCount;
    Point* previousPoints;
    size_t previousPointsCount;
    Point center;
    float scaleX;
    float scaleY;
    int segments;
    float thickness;
    color_t fillColor;
} Strip;

typedef struct {
    void* shape;
    void (*init)(void*);
    void (*set_points)(void*, Point*, size_t);
    Point* (*get_points)(void*);
    void (*set_thickness)(void*, float);
    float (*get_thickness)(const void*);
    void (*set_scaleX)(void*, float);
    float (*get_scaleX)(const void*);
    void (*set_scaleY)(void*, float);
    float (*get_scaleY)(const void*);
    void (*set_center)(void*, Point);
    Point (*get_center)(const void*);
    void (*set_segments)(void*, int);
    int (*get_segments)(const void*);
    void (*set_lod)(void*, float);
    float (*get_lod)(const void*);
    void (*set_fill_color)(void*, color_t);
    color_t (*get_fill_color)(const void*);
    void (*resolve)(void*, float, float);
    void (*destroy)(void*);
} ShapeInterface;

// Initialization functions
void shape_init(Shape* shape);
void fan_init(Fan* fan, Point origin, float scale, int segments, color_t fillColor);
void fan2_init(Fan* fan, Point origin, float scaleX, float scaleY, int segments, color_t fillColor);
void circle_init(Circle* circle, Point origin, float scale, float lod, color_t fillColor);
void strip_init(Strip* strip, Point origin, float scaleX, float scaleY, float thickness, int segments, color_t fillColor);

// Common functions for the Shape interface
void set_points(void* shape, Point* points, size_t count);
Point* get_points(void* shape);
void set_thickness(void* shape, float thickness);
float get_thickness(const void* shape);
void set_scaleX(void* shape, float scaleX);
float get_scaleX(const void* shape);
void set_scaleY(void* shape, float scaleY);
float get_scaleY(const void* shape);
void set_center(void* shape, Point center);
Point get_center(const void* shape);
void set_segments(void* shape, int segments);
int get_segments(const void* shape);
void set_lod(void* shape, float lod);
float get_lod(const void* shape);
void set_fill_color(void* shape, color_t fillColor);
color_t get_fill_color(const void* shape);
void resolve(void* shape, float stickX, float stickY);
void destroy(void* shape);

// Shape-specific function table initialization
void init_shape_interface(ShapeInterface* interface, void* shape);

#endif // SHAPE_H
