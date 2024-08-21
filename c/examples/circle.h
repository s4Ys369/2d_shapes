#ifndef CIRCLE_H
#define CIRCLE_H

#include <libdragon.h>
#include "control.h"

Shape* circle;

void create_circle(){
  circle = (Shape*)malloc_uncached(sizeof(Shape));
  circle_init(circle, screenCenter, 20.0f, 0.05f, RED); 
}

void circle_draw(){
  currShape = circle;

  // Get ellipse points and store them in currPoints
  render_get_ellipse_points(currPoints, currCenter, currRadiusX, currRadiusY, currSegments);

  // Resolve the shape based on joystick inputs
  resolve(currShape, stickX, stickY);

  // Update current shape properties
  currCenter = get_center(currShape);
  currRadiusX = get_scaleX(currShape);
  currRadiusY = get_scaleY(currShape);
  currSegments = get_segments(currShape);
  currLOD = get_lod(currShape);

  // Adjust LOD if necessary
  if (currLOD < ((float)currSegments * 0.01f)) {
    currLOD = ((float)currSegments * 0.01f);
  }

  // Set render color and draw the circle
  currShapeColor = get_fill_color(currShape);
  set_render_color(currShapeColor);
  draw_circle(currCenter.x, currCenter.y, currRadiusX, currRadiusY, currAngle, currLOD);

  // Get the current points from the shape
  currPoints = get_points(currShape);
  set_points(currShape, currPoints);
}

#endif // CIRCLE_H