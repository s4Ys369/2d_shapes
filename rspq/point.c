#include <libdragon.h>
#include "point.h"
#include "vec2_helper.h"

vec2_t point_to_vec2(const Point* p) {
    return (vec2_t){ .v[0] = p->x, .v[1] = p->y };
}

Point vec2_to_point(const vec2_t* v) {
    return (Point){ .x = v->v[0], .y = v->v[1] };
}

// Constructors
Point point_new(float x, float y) {
    Point p = { x, y };
    return p;
}

Point point_default() {
    return point_new(0, 0);
}

// Operations
// Add two Points
Point point_add(const Point* p1, const Point* p2) {
    vec2_t v1 = point_to_vec2(p1);
    vec2_t v2 = point_to_vec2(p2);
    vec2_t result = vec2_add(&v1, &v2);
    return vec2_to_point(&result);
}

// Subtract two Points
Point point_subtract(const Point* p1, const Point* p2) {
    vec2_t v1 = point_to_vec2(p1);
    vec2_t v2 = point_to_vec2(p2);
    vec2_t result = vec2_subtract(&v1, &v2);
    return vec2_to_point(&result);
}

// Multiply a Point by a scalar
Point point_multiply(const Point* p, float scalar) {
    vec2_t v = point_to_vec2(p);
    vec2_t result = vec2_multiply(&v, scalar);
    return vec2_to_point(&result);
}

// Normalize a Point
Point point_normalized(const Point* p) {
    vec2_t v = point_to_vec2(p);
    vec2_t result = vec2_normalized(&v);
    return vec2_to_point(&result);
}

// Scale a Point
Point point_set_mag(const Point* p, float newMag) {
    vec2_t v = point_to_vec2(p);
    vec2_t normalized = vec2_normalized(&v);
    vec2_t result = vec2_multiply(&normalized, newMag);
    return vec2_to_point(&result);
}

// Member functions
void point_add_in_place(Point* p, const Point* v) {
    p->x += v->x;
    p->y += v->y;
}

Point point_sum(const Point* v1, const Point* v2) {
    return point_add(v1, v2);
}

Point point_sub(const Point* v1, const Point* v2) {
    return point_subtract(v1, v2);
}

// Calculate heading of a Point
float point_heading(const Point* p) {
    vec2_t v = point_to_vec2(p);
    return vec2_heading(&v);
}

Point point_from_angle(float angle) {
    vec2_t v = vec2_from_angle(angle);
    return vec2_to_point(&v);
}



Point point_copy(const Point* p) {
    return point_new(p->x, p->y);
}

Point point_scale(const Point* center, const Point* point, float scale) {
    Point direction = point_subtract(point, center);
    Point scaled = point_multiply(&direction, scale);
    return point_add(center, &scaled);
}

// Translate a Point
Point point_translate(const Point* p, float dx, float dy) {
    vec2_t v = point_to_vec2(p);
    vec2_t translation = { .v[0] = dx, .v[1] = dy };
    vec2_t result = vec2_add(&v, &translation);
    return vec2_to_point(&result);
}

// Rotate a Point around another Point
void point_rotate(Point* p, const Point* center, float angle) {
    vec2_t v = point_to_vec2(p);
    vec2_t c = point_to_vec2(center);
    vec2_t result = vec2_rotate(&v, &c, angle);
    *p = vec2_to_point(&result);
}

Point point_transform(const Point* point, float angle, float width) {
    Point rotated = point_from_angle(angle);
    Point scaled = point_multiply(&rotated, width);
    return point_add(point, &scaled);
}

// Lerp between two Points
Point point_lerp(const Point* p1, const Point* p2, float t) {
    vec2_t v1 = point_to_vec2(p1);
    vec2_t v2 = point_to_vec2(p2);
    vec2_t result = vec2_lerp(&v1, &v2, t);
    return vec2_to_point(&result);
}

// Check if a Point is in a triangle
bool point_in_triangle(const Point* P, const Point* A, const Point* B, const Point* C) {
    vec2_t p = point_to_vec2(P);
    vec2_t a = point_to_vec2(A);
    vec2_t b = point_to_vec2(B);
    vec2_t c = point_to_vec2(C);
    return vec2_in_triangle(&p, &a, &b, &c);
}

// Cross product of two Points
float point_cross(const Point* p1, const Point* p2) {
    vec2_t v1 = point_to_vec2(p1);
    vec2_t v2 = point_to_vec2(p2);
    return vec2_cross(&v1, &v2);
}

// Dot product of two Points
float point_dot(const Point* p1, const Point* p2) {
    vec2_t v1 = point_to_vec2(p1);
    vec2_t v2 = point_to_vec2(p2);
    return vec2_dot(&v1, &v2);
}

// Epsilon test for collinearity
float point_epsilon_test(const Point* A, const Point* B, const Point* C) {
    vec2_t a = point_to_vec2(A);
    vec2_t b = point_to_vec2(B);
    vec2_t c = point_to_vec2(C);
    return vec2_epsilon_test(&a, &b, &c);
}

void point_move(Point* p, float stickX, float stickY) {
    p->x += stickX;
    p->y += stickY;
}

void rotate_line_point(Point* p, const Point* center, float cos_angle, float sin_angle) {
    float tx = p->x - center->x;
    float ty = p->y - center->y;
    p->x = center->x + (tx * cos_angle - ty * sin_angle);
    p->y = center->y + (tx * sin_angle + ty * cos_angle);
}

// Function to initialize a PointArray
void init_point_array(PointArray* array) {
    if (array) {
        array->points = (Point*)malloc(sizeof(Point));
        if (array->points) {
            array->count = 0;
        } else {
            array->count = 0;
        }
    }
}

// Function to initialize a PointArray from existing points
void init_point_array_from_points(PointArray* array, Point* points, size_t count) {
    array->points = (Point*)malloc(sizeof(Point) * count);
    if (array->points == NULL) {
        // Handle memory allocation failure
        debugf("Point allocation failed\n");
        return;
    }
    memcpy(array->points, points, sizeof(Point) * count);
    array->count = count;
}

// Function to add a point to a PointArray
void add_point(PointArray* array, float x, float y) {
    Point* new_points = (Point*)realloc(array->points, sizeof(Point) * (array->count + 1));
    if (new_points == NULL) {
        debugf("Point reallocation failed\n");
        return;
    }
    array->points = new_points;
    array->points[array->count].x = x;
    array->points[array->count].y = y;
    array->count++;
}

// Function to add an existing point to the PointArray
void add_existing_point(PointArray* array, Point p) {
    Point* new_points = (Point*)realloc(array->points, sizeof(Point) * (array->count + 1));
    if (new_points == NULL) {
        debugf("Point reallocation failed\n");
        return;
    }
    array->points = new_points;
    array->points[array->count] = p;
    array->count++;
}

void calculate_array_center(const PointArray* points, Point* center) {
    center->x = 0.0f;
    center->y = 0.0f;

    for (size_t i = 0; i < points->count; ++i) {
        center->x += points->points[i].x;
        center->y += points->points[i].y;
    }

    center->x /= points->count;
    center->y /= points->count;
}


// Function to free a PointArray
void free_point_array(PointArray* array) {
    free_uncached(array);
}
