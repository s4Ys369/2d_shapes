#ifndef SHAPE_H
#define SHAPE_H

#include <libdragon.h>
#include "Point.h"
#include "Render.h"
#include "Utils.h"

class Shape {
public:

    Shape();
    Shape(Point origin, float scale, int segments, color_t shapeColor);
    Shape(Point origin, float scale, float lod, color_t shapeColor);
    Shape(Point origin, float scaleX, float scaleY, int segments, color_t shapeColor);

    const std::vector<Point>& get_points() const { return currPoints; }
    void set_points(const std::vector<Point>& points) { this->currPoints = points; };

    void set_thickness(float thickness) { this->scaleX = thickness; }
    float get_thickness() const { return scaleX*scaleY; }

    void set_scaleX(float scaleX) { this->scaleX = scaleX; }
    float get_scaleX() const { return scaleX; }

    void set_scaleY(float scaleY) { this->scaleY = scaleY; }
    float get_scaleY() const { return scaleY; }

    void set_center(const Point& center) { this->center = center; }
    Point get_center() { return center; }

    void set_segments(int segments) { this->segments = segments; }
    int get_segments() const { return segments; }

    void set_lod(float lod) { this->lod = lod; }
    float get_lod() const { return lod; }

    void set_shape_fill_color(color_t shapeColor) { this->shapeColor = shapeColor; }
    color_t get_shape_fill_color() const { return shapeColor; }

    void resolve(float stickX, float stickY);

private:
    std::vector<Point> currPoints;
    std::vector<Point> previousPoints;
    Point center;
    float scaleX;
    float scaleY;
    int segments;
    float lod;
    color_t shapeColor;
};

#endif // SHAPE_H