#include <libdragon.h>
#include "point.h"
#include "Utils.h"

// Constructors
Point point_new(float x, float y) {
    Point p = { x, y };
    return p;
}

Point point_default() {
    return point_new(0, 0);
}

// Operations
Point point_subtract(const Point* p1, const Point* p2) {
    return point_new(p1->x - p2->x, p1->y - p2->y);
}

Point point_add(const Point* p1, const Point* p2) {
    return point_new(p1->x + p2->x, p1->y + p2->y);
}

Point point_multiply(const Point* p, float scalar) {
    return point_new(p->x * scalar, p->y * scalar);
}

Point point_normalized(const Point* p) {
    float length = sqrt(p->x * p->x + p->y * p->y);
    return point_new(p->x / length, p->y / length);
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

float point_heading(const Point* p) {
    return atan2f(p->y, p->x);
}

Point point_from_angle(float angle) {
    return point_new(fm_cosf(angle), fm_sinf(angle));
}

float point_magnitude(const Point* p) {
    return sqrtf(p->x * p->x + p->y * p->y);
}

void point_normalize(Point* p) {
    float length = point_magnitude(p);
    if(length != 0) {
        p->x /= length;
        p->y /= length;
    }
}

Point point_set_mag(Point* p, float newMag) {
    Point normalized = point_normalized(p);
    return point_multiply(&normalized, newMag);
}

Point point_copy(const Point* p) {
    return point_new(p->x, p->y);
}

Point point_scale(const Point* center, const Point* point, float scale) {
    Point direction = point_subtract(point, center);
    Point scaled = point_multiply(&direction, scale);
    return point_add(center, &scaled);
}

Point point_translate(Point p, float dx, float dy) {
    return point_new(p.x + dx, p.y + dy);
}

void point_rotate(Point* p, const Point* center, float angle) {
    float s = fm_sinf(angle);
    float c = fm_cosf(angle);
    
    p->x -= center->x;
    p->y -= center->y;

    float xnew = p->x * c - p->y * s;
    float ynew = p->x * s + p->y * c;

    p->x = xnew + center->x;
    p->y = ynew + center->y;
}

Point point_transform(const Point* point, float angle, float width) {
    Point rotated = point_from_angle(angle);
    Point scaled = point_multiply(&rotated, width);
    return point_add(point, &scaled);
}

float point_cross(const Point* p1, const Point* p2) {
    return p1->x * p2->y - p1->y * p2->x;
}

float point_dot(const Point* p1, const Point* p2) {
    return p1->x * p2->x + p1->y * p2->y;
}

float point_epsilon_test(const Point* A, const Point* B, const Point* C) {
    float epsilon = 1e-6f;
    Point AB = point_subtract(B, A);
    Point AC = point_subtract(C, A);
    float cross = point_cross(&AB, &AC);
    return fabsf(cross) < epsilon;
}

bool point_point_in_triangle(const Point* P, const Point* A, const Point* B, const Point* C) {
    Point v0 = point_subtract(C, A);
    Point v1 = point_subtract(B, A);
    Point v2 = point_subtract(P, A);

    float dot00 = point_dot(&v0, &v0);
    float dot01 = point_dot(&v0, &v1);
    float dot02 = point_dot(&v0, &v2);
    float dot11 = point_dot(&v1, &v1);
    float dot12 = point_dot(&v1, &v2);

    float invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0) && (v >= 0) && (u + v < 1);
}

void point_move(Point* p, float stickX, float stickY) {
    p->x += stickX;
    p->y += stickY;
}
