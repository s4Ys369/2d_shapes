#ifndef SHAPE_H
#define SHAPE_H

#include <libdragon.h>
#include "Point.h"
#include "Render.h"
#include "Utils.h"

class Shape {
public:
    Shape(Point origin);

    const std::vector<Point>& get_points() const { return currPoints; }
    void set_scale(float scale) { this->scale = scale; }
    float get_scale() const { return scale; }

    void set_center(const Point& center) { this->center = center; }
    static Point get_center() { return center; }

    void set_segments(int segments) { this->segments = segments; }
    int get_segments() const { return segments; }
    void resolve(float stickX, float stickY);

private:
    std::vector<Point> currPoints;
    std::vector<Point> previousPoints;
    float scale;
    Point center;
    int segments;
};

#endif // SHAPE_H