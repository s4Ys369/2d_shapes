#ifndef POINT_H
#define POINT_H

#include <libdragon.h>
#include <vector>

class Point {
public:
    float x, y;

    Point(float x = 0, float y = 0);

    Point operator-(const Point& other) const {
        return {x - other.x, y - other.y};
    }

    Point operator+(const Point& other) const {
        return {x + other.x, y + other.y};
    }

    Point operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    Point normalized() const {
        float length = std::sqrt(x * x + y * y);
        return {x / length, y / length};
    }

    void add(const Point& v);
    static Point sum(const Point& v1, const Point& v2);
    static Point sub(const Point& v1, const Point& v2);
    float heading() const;
    static Point from_angle(float angle);
    float magnitude() const;
    void normalize();
    Point& set_mag(float newMag);
    Point copy() const;
    static Point scale(const Point& center, const Point& point, float scale);
    static Point translate(Point p, float dx, float dy);
    void rotate(Point center, float angle);
    static Point transform(const Point& point, float angle, float width);
    static float epsilon_test(const Point& A, const Point& B, const Point& C);
    static bool point_in_triangle(const Point& P, const Point& A, const Point& B, const Point& C);
    void move(float stickX, float stickY);
};

#endif // POINT_H