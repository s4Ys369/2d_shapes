#include <libdragon.h>
#include "Point.h"
#include "Utils.h"

// Point, the API's internal Vector2f

// Constructor that initializes a Point object with given x and y coordinates
Point::Point(float x, float y) : x(x), y(y) {}

// Adds the coordinates of another Point to the current Point
void Point::add(const Point& v) {
    x += v.x;
    y += v.y;
}

// Returns a new Point that is the result of adding two Point objects
Point Point::sum(const Point& v1, const Point& v2) {
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
    float mag = magnitude();
    if (mag != 0) {
        normalize();
        x *= newMag;
        y *= newMag;
    }
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

// Rotates a Point around a given center by a given angle
Point Point::rotate(Point p, Point center, float angle) {
    float s = fm_sinf(angle);
    float c = fm_cosf(angle);
    
    // Translate point to origin
    float xnew = p.x - center.x;
    float ynew = p.y - center.y;

    // Rotate point
    float xrot = xnew * c - ynew * s;
    float yrot = xnew * s + ynew * c;

    // Translate point back
    xrot += center.x;
    yrot += center.y;

    return Point(xrot, yrot);
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

// Function to move point around the screen using the Control Stick
Point Point::move(float stickX, float stickY) {
    Point targetPos = Point(x,y);

    // Apply deadzone to the joystick inputs
    float adjustedX = apply_deadzone(stickX);
    float adjustedY = apply_deadzone(stickY);

    // Normalize the joystick input to get the direction
    Point direction(adjustedX, -adjustedY);
    direction.normalize();

    // Limit movement to inside screen with offset
    float offset = 32.0f;
    float width = display_get_width();
    float height = display_get_height();
    Point screenCenter = {width / 2.0f, height / 2.0f};

    if (x < offset) {
        x = offset;
        targetPos = screenCenter;
    }
    if (x > width - offset) {
        x = width - offset;
        targetPos = screenCenter;
    }
    if (y < offset) {
        y = offset;
        targetPos = screenCenter;
    }
    if (y > height - offset) {
        y = height - offset;
        targetPos = screenCenter;
    }

    // Determine the target position based on the direction and a fixed magnitude
    float move_mag = 3.5f;
    targetPos = Point::sum(*this, direction.set_mag(move_mag));

    return Point::sum(*this, targetPos);
}