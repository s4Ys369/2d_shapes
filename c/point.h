#ifndef POINT_H
#define POINT_H

#include <libdragon.h>
#include <math.h>
#include <stdbool.h>
#include "rspq/vec_gfx.h"

typedef struct {
    float x, y;
} Point;

typedef struct {
    Point* points;
    size_t count;
} PointArray;

#define MAX_BATCH_SIZE 128

// Constructors
Point point_new(float x, float y);
Point point_default();

// Operations
Point point_subtract(const Point* p1, const Point* p2);
Point point_add(const Point* p1, const Point* p2);
Point point_multiply(const Point* p, float scalar);
Point point_normalized(const Point* p);

// Transform functions
void point_add_in_place(Point* p, const Point* v);
Point point_sum(const Point* v1, const Point* v2);
Point point_sub(const Point* v1, const Point* v2);
float point_heading(Point p);
Point point_from_angle(float angle);
float point_magnitude(const Point* p);
void point_normalize(Point* p);
Point point_set_mag(Point* p, float newMag);
Point point_copy(const Point* p);
Point point_scale(const Point* center, const Point* point, float scale);
void point_scale_batch(const Point* centers, const Point* points, float* scales, Point* scaled_points, uint32_t count);
Point point_translate(Point p, float dx, float dy);
void point_rotate(Point* p, const Point* center, float angle);
void point_rotate_batch(Point* points, const Point* center, float* angles, int count);
Point point_transform(const Point* point, float angle, float width);
void point_transform_4x4_batch(Point* points, const mtx4x4_t *mat, int count);
float point_cross(const Point* p1, const Point* p2);
float point_dot(const Point* p1, const Point* p2);
float point_epsilon_test(const Point* A, const Point* B, const Point* C);
bool point_in_triangle(const Point* P, const Point* A, const Point* B, const Point* C);
void point_move(Point* p, float stickX, float stickY);
void rotate_line_point(Point* p, const Point* center, float cos_angle, float sin_angle);
Point point_lerp(Point* p1, Point* p2, float t);

// PointArray functions
void init_point_array(PointArray* array);
void init_point_array_from_points(PointArray* array, Point* points, size_t count);
void add_point(PointArray* array, float x, float y);
void add_existing_point(PointArray* array, Point p);
void calculate_array_center(const PointArray* points, Point* center);
void free_point_array(PointArray* array);

void constrain_dist_rsp(Point* positions, Point* anchors, float* constraints, int count);

#endif // POINT_H
