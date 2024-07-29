#include <libdragon.h>
#include "Point.h"

// Point, the API's internal Vector2f

// Constructor that initializes a Point object with given x and y coordinates
Point::Point(float x, float y) : x(x), y(y) {}

// Adds the coordinates of another Point to the current Point
void Point::add(const Point& v) {
    x += v.x;
    y += v.y;
}

// Returns a new Point that is the result of adding two Point objects
Point Point::add(const Point& v1, const Point& v2) {
    return Point(v1.x + v2.x, v1.y + v2.y);
}

// Returns a new Point that is the result of subtracting one Point from another
Point Point::sub(const Point& v1, const Point& v2) {
    return Point(v1.x - v2.x, v1.y - v2.y);
}


// Computes the angle (heading) of the vector from the origin to the current Point, in radians
float Point::heading() const {
    return fm_atan2f(y, x);
}

// Creates a Point representing a vector with unit length in the direction of the given angle
Point Point::from_angle(float angle) {
    return Point(fm_cosf(angle), fm_sinf(angle));
}

// Calculates the magnitude (length) of the vector represented by the current Point
float Point::magnitude() const {
    return sqrtf(x * x + y * y);
}

// Normalizes the vector represented by the current Point
void Point::normalize() {
    float mag = magnitude();
    if (mag != 0) {
        x /= mag;
        y /= mag;
    }
}

// Sets the magnitude of the vector represented by the current Point to a new value
Point& Point::set_mag(float newMag) {
    normalize();
    x *= newMag;
    y *= newMag;
    return *this;
}

// Creates a copy of the current Point
Point Point::copy() const {
    return Point(x, y);
}

// Scales a point outward from a center by the given scale factor
Point Point::scale(const Point& center, const Point& point, float scale) {
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    return Point(center.x + dx * scale, center.y + dy * scale);
}

// Translates a Point by adding dx and dy to its coordinates
Point Point::translate(Point p, float dx, float dy) {
    p.x += dx;
    p.y += dy;
    return p;
}

// Rotates a Point around the origin by a given angle
Point Point::rotate(Point p, float angle) {
    float s = fm_sinf(angle);
    float c = fm_cosf(angle);
    float xnew = p.x * c - p.y * s;
    float ynew = p.x * s + p.y * c;
    p.x = xnew;
    p.y = ynew;
    return p;
}

// Transforms a Point by translating it along a direction given by an angle and distance
Point Point::transform(const Point& point, float angle, float width) {
    Point transformed;
    transformed.x = point.x + fm_cosf(angle) * width;
    transformed.y = point.y + fm_sinf(angle) * width;
    return transformed;
}

// Performs an epsilon test to determine the orientation of the point C relative to the line segment from A to B
float Point::epsilon_test(const Point& A, const Point& B, const Point& C) {
    return (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);
}


// Checks if a point P is inside the triangle defined by points A, B, and C
bool Point::point_in_triangle(const Point& P, const Point& A, const Point& B, const Point& C) {
    float ax = C.x - B.x, ay = C.y - B.y;
    float bx = A.x - C.x, by = A.y - C.y;
    float cx = B.x - A.x, cy = B.y - A.y;
    float apx = P.x - A.x, apy = P.y - A.y;
    float bpx = P.x - B.x, bpy = P.y - B.y;
    float cpx = P.x - C.x, cpy = P.y - C.y;

    float aCROSSbp = ax * bpy - ay * bpx;
    float cCROSSap = cx * apy - cy * apx;
    float bCROSScp = bx * cpy - by * cpx;

    return ((aCROSSbp >= 0) && (bCROSScp >= 0) && (cCROSSap >= 0));
}