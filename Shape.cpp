#include <libdragon.h>
#include "Point.h"
#include "Render.h"
#include "Shape.h"
#include "Utils.h"

Shape::Shape() : scale(1.0f), center({0.0f, 0.0f}), segments(1) {}

Shape::Shape(Point origin, float scale, int segments)
    : center(origin),
        scale(scale),
        segments(segments) {}

void Shape::resolve(float stickX, float stickY) {
  Point currPos =  center;
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
  Point center = {width/2,height/2};

  if(currPos.x < offset){currPos.x = offset; targetPos = center;}
  if(currPos.x > width - offset){currPos.x = width - offset; targetPos = center;}
  if(currPos.y < offset){currPos.y = offset; targetPos = center;}
  if(currPos.y > height - offset){currPos.y = height - offset; targetPos = center;}

  // Determine the target position based on the direction and a fixed magnitude
  float move_mag = 3.5;
  targetPos = Point::add(currPos, direction.set_mag(move_mag));

  center = targetPos;
  //debugf("X %.1f\nY %.1f\n", targetPos.x, targetPos.y);
}

