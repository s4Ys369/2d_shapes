#include <libdragon.h>
#include "point.h"
#include "render.h"
#include "shapes.h"
#include "utils.h"

// Initialization functions
void shape_init(Shape* shape) {
    if (shape == NULL) {
        return;
    }

    shape->center = point_default();
    shape->scaleX = 1.0f;
    shape->scaleY = 1.0f;
    shape->segments = 1;
    shape->lod = 1.0f;
    shape->fillColor = BLACK;
    shape->previousPoints = (PointArray*)malloc(sizeof(PointArray));
    shape->previousPoints->count = 0;
    shape->currPoints = (PointArray*)malloc(sizeof(PointArray));
    shape->currPoints->count = 0;
    add_existing_point(shape->currPoints, shape->center);

}

void circle_init(Shape* circle, Point origin, float scale, float lod, color_t fillColor) {
    shape_init(circle);
    circle->center = origin;
    circle->scaleX = scale;
    circle->lod = lod;
    circle->fillColor = fillColor;
}

void fan_init(Shape* fan, Point origin, float scale, int segments, color_t fillColor) {
    shape_init(fan);
    fan->center = origin;
    fan->scaleX = scale;
    fan->scaleY = scale;
    fan->segments = segments;
    fan->fillColor = fillColor;
}

void fan2_init(Shape* fan, Point origin, float scaleX, float scaleY, int segments, color_t fillColor) {
    shape_init(fan);
    fan->center = origin;
    fan->scaleX = scaleX;
    fan->scaleY = scaleY;
    fan->segments = segments;
    fan->fillColor = fillColor;
}

void strip_init(Shape* strip, Point origin, float scaleX, float scaleY, float thickness, int segments, color_t fillColor) {
    shape_init(strip);
    strip->center = origin;
    strip->scaleX = scaleX;
    strip->scaleY = scaleY;
    strip->lod = thickness;
    strip->segments = segments;
    strip->fillColor = fillColor;
}


// Common functions for shapes
void set_points(Shape* shape, PointArray* points) {

    // Free old points if they exist
    if (shape->currPoints != NULL) {
        free(shape->currPoints->points);
    }

    // Copy new points
    memcpy(shape->currPoints->points, points->points, sizeof(Point) * points->count);
    shape->currPoints->count = points->count;
}

PointArray* get_points(Shape* shape) {
    return shape->currPoints;
}

void set_thickness(Shape* shape, float thickness) {
    shape->lod = thickness;
}

float get_thickness(const Shape* shape) {
    return shape->lod;
}

void set_scaleX(Shape* shape, float scaleX) {
    shape->scaleX = scaleX;
}

float get_scaleX(const Shape* shape) {
    return shape->scaleX;
}

void set_scaleY(Shape* shape, float scaleY) {
    shape->scaleY = scaleY;
}

float get_scaleY(const Shape* shape) {
    return shape->scaleY;
}

void set_center(Shape* shape, Point center) {
    shape->center = center;
}

Point get_center(const Shape* shape) {
    return shape->center;
}

void set_segments(Shape* shape, int segments) {
    shape->segments = segments;
}

int get_segments(const Shape* shape) {
    return shape->segments;
}

void set_lod(Shape* shape, float lod) {
    shape->lod = lod;
}

float get_lod(const Shape* shape) {
    return shape->lod;
}

void set_fill_color(Shape* shape, color_t fillColor) {
    shape->fillColor = fillColor;
}

color_t get_fill_color(const Shape* shape) {
    return shape->fillColor;
}

// Function to move shape around the screen using the Control Stick
void resolve(Shape* shape, float stickX, float stickY) {
    if (shape == NULL || shape->currPoints == NULL || shape->currPoints->points == NULL) {
        debugf("Invalid shape or points\n");
        return;
    }

    Point currPos = get_center(shape);
    Point targetPos = currPos;

    // Apply deadzone to the joystick inputs
    float adjustedX = apply_deadzone(stickX);
    float adjustedY = apply_deadzone(stickY);

    // Normalize the joystick input to get the direction
    Point direction = point_new(adjustedX, -adjustedY);
    point_normalize(&direction);

    // Limit movement to inside screen with offset
    float offset = 32.0f;
    float width = display_get_width();
    float height = display_get_height();
    Point screenCenter = {width / 2.0f, height / 2.0f};

    if (currPos.x < offset) {
        currPos.x = offset;
        targetPos = screenCenter;
    }
    if (currPos.x > width - offset) {
        currPos.x = width - offset;
        targetPos = screenCenter;
    }
    if (currPos.y < offset) {
        currPos.y = offset;
        targetPos = screenCenter;
    }
    if (currPos.y > height - offset) {
        currPos.y = height - offset;
        targetPos = screenCenter;
    }

    // Determine the target position based on the direction and a fixed magnitude
    float move_mag = 3.5f;
    direction = point_set_mag(&direction, move_mag);
    targetPos = point_sub(&currPos, &direction);

    set_center(shape, targetPos);
    // debugf("X %.1f\nY %.1f\n", targetPos.x, targetPos.y);
}

void destroy(Shape* shape) {
    if (shape->currPoints != NULL) {
        free(shape->currPoints);
    }
    if (shape->previousPoints != NULL) {
        free(shape->previousPoints);
    }
}


