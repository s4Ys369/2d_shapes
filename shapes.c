#include <libdragon.h>
#include "point.h"
#include "Render.h"
#include "shape.h"
#include "Utils.h"

// Initialization functions
void shape_init(Shape* shape) {
    memset(shape, 0, sizeof(Shape));
    shape->center = (Point){0, 0};
    shape->scaleX = 1.0f;
    shape->scaleY = 1.0f;
    shape->segments = 1;
    shape->lod = 1.0f;
    shape->fillColor = BLACK;
    shape->currPoints = (Point*)malloc(sizeof(Point));
    if (shape->currPoints != NULL) {
        shape->currPoints[0] = shape->center;
        shape->currPointsCount = 1;
    }
}

void circle_init(Circle* circle, Point origin, float scale, float lod, color_t fillColor) {
    shape_init((Shape*)circle);
    circle->center = origin;
    circle->scale = scale;
    circle->lod = lod;
    circle->fillColor = fillColor;
}

void fan_init(Fan* fan, Point origin, float scale, int segments, color_t fillColor) {
    shape_init((Shape*)fan);
    fan->center = origin;
    fan->scaleX = scale;
    fan->scaleY = scale;
    fan->segments = segments;
    fan->fillColor = fillColor;
}

void fan2_init(Fan* fan, Point origin, float scaleX, float scaleY, int segments, color_t fillColor) {
    shape_init((Shape*)fan);
    fan->center = origin;
    fan->scaleX = scaleX;
    fan->scaleY = scaleY;
    fan->segments = segments;
    fan->fillColor = fillColor;
}

void strip_init(Strip* strip, Point origin, float scaleX, float scaleY, float thickness, int segments, color_t fillColor) {
    shape_init((Shape*)strip);
    strip->center = origin;
    strip->scaleX = scaleX;
    strip->scaleY = scaleY;
    strip->lod = thickness;
    strip->segments = segments;
    strip->fillColor = fillColor;
}


// Common functions for the Shape interface
void set_points(void* shape, Point* points, size_t count) {
    Shape* s = (Shape*)shape;
    Point* newPoints = (Point*)realloc(s->currPoints, sizeof(Point) * count);
    if (newPoints != NULL) {
        s->currPoints = newPoints;
        memcpy(s->currPoints, points, sizeof(Point) * count);
        s->currPointsCount = count;
    } else {
        debugf("Point reallocation faliure\n");
    }
}

Point* get_points(void* shape) {
    return ((Shape*)shape)->currPoints;
}

void set_thickness(void* shape, float thickness) {
    ((Shape*)shape)->scaleX = thickness;
}

float get_thickness(const void* shape) {
    return ((Shape*)shape)->lod;
}

void set_scaleX(void* shape, float scaleX) {
    ((Shape*)shape)->scaleX = scaleX;
}

float get_scaleX(const void* shape) {
    return ((Shape*)shape)->scaleX;
}

void set_scaleY(void* shape, float scaleY) {
    ((Shape*)shape)->scaleY = scaleY;
}

float get_scaleY(const void* shape) {
    return ((Shape*)shape)->scaleY;
}

void set_center(void* shape, Point center) {
    ((Shape*)shape)->center = center;
}

Point get_center(const void* shape) {
    return ((Shape*)shape)->center;
}

void set_segments(void* shape, int segments) {
    ((Shape*)shape)->segments = segments;
}

int get_segments(const void* shape) {
    return ((Shape*)shape)->segments;
}

void set_lod(void* shape, float lod) {
    ((Shape*)shape)->lod = lod;
}

float get_lod(const void* shape) {
    return ((Shape*)shape)->lod;
}

void set_fill_color(void* shape, color_t fillColor) {
    ((Shape*)shape)->fillColor = fillColor;
}

color_t get_fill_color(const void* shape) {
    return ((Shape*)shape)->fillColor;
}

// Function to move shape around the screen using the Control Stick
void resolve(void* shape, float stickX, float stickY) {
    Point currPos = ((Shape*)shape)->center;
    Point targetPos = currPos;

    // Apply deadzone to the joystick inputs
    float adjustedX = apply_deadzone(stickX);
    float adjustedY = apply_deadzone(stickY);

    // Normalize the joystick input to get the direction
    Point direction(adjustedX, -adjustedY);
    direction.normalize();// FIXME

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
    targetPos = Point::sub(currPos, direction.set_mag(move_mag));//FIXME

    set_center(((Shape*)shape), targetPos);
    // debugf("X %.1f\nY %.1f\n", targetPos.x, targetPos.y);
}

void destroy(void* shape) {
    Shape* s = (Shape*)shape;
    if (s->currPoints != NULL) {
        free(s->currPoints);
    }
    if (s->previousPoints != NULL) {
        free(s->previousPoints);
    }
}

// Shape-specific function table initialization
void init_shape_interface(ShapeInterface* interface, void* shape) {
    interface->shape = shape;
    interface->init = shape_init;
    interface->set_points = set_points;
    interface->get_points = get_points;
    interface->set_thickness = set_thickness;
    interface->get_thickness = get_thickness;
    interface->set_scaleX = set_scaleX;
    interface->get_scaleX = get_scaleX;
    interface->set_scaleY = set_scaleY;
    interface->get_scaleY = get_scaleY;
    interface->set_center = set_center;
    interface->get_center = get_center;
    interface->set_segments = set_segments;
    interface->get_segments = get_segments;
    interface->set_lod = set_lod;
    interface->get_lod = get_lod;
    interface->set_fill_color = set_fill_color;
    interface->get_fill_color = get_fill_color;
    interface->resolve = resolve;
    interface->destroy = destroy;
}

