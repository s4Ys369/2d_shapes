#include <libdragon.h>
#include "Point.h"
#include "Render.h"
#include "Shape.h"
#include "Utils.h"

// Default constructor
Shape::Shape() : center({100.0f, 100.0f}), scaleX(1.0f), scaleY(1.0f), segments(1), lod(1.0f), shapeColor(BLACK) {}

// Fan constructor
Shape::Shape(Point origin, float scaleX, int segments, color_t shapeColor)
    : center(origin),
        scaleX(scaleX),
        scaleY(scaleX),
        segments(segments),
        shapeColor(shapeColor) {}

// Ellipse/line constructor
Shape::Shape(Point origin, float scaleX, float lod, color_t shapeColor)
    : center(origin),
        scaleX(scaleX),
        scaleY(scaleX),
        segments(3), // base amount to draw a triangle
        lod(lod),
        shapeColor(shapeColor) {}

// Strip constructor
Shape::Shape(Point origin, float scaleX, float scaleY, int segments, color_t shapeColor)
    : center(origin),
        scaleX(scaleX),
        scaleY(scaleY),
        segments(segments),
        shapeColor(shapeColor) {}

// Function to move shape around the screen using the Control Stick
void Shape::resolve(float stickX, float stickY) {
    Point currPos = center;
    Point targetPos = currPos;

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
    targetPos = Point::add(currPos, direction.set_mag(move_mag));

    set_center(targetPos);
    // debugf("X %.1f\nY %.1f\n", targetPos.x, targetPos.y);
}

