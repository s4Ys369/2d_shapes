#ifndef QUAD_H
#define QUAD_H

#include <libdragon.h>
#include "control.h"
#include "globals.h"

Shape* quad;

void create_quad(){
// Quad as a strip
  quad = (Shape*)malloc_uncached(sizeof(Shape));
  strip_init(quad, screenCenter, 20.0f, 20.0f, 0.01f, 1, DARK_GREEN);
}

void quad_draw(){
  // Update current shape properties
  currShape = quad;
  resolve(quad, stickX, stickY);
  currCenter = get_center(currShape);
  currRadiusX = get_scaleX(currShape);
  currRadiusY = get_scaleY(currShape);
  currThickness = get_thickness(currShape); // FIXME: Isn't utilized in example
  currShapeColor = get_fill_color(currShape);
  currSegments = get_segments(currShape);
  set_render_color(currShapeColor);
  draw_line(
    currCenter.x-currRadiusX, currCenter.y-currRadiusY, 
    currCenter.x+currRadiusX, currCenter.y+currRadiusY, 
    currAngle,
    currThickness
  );
}

#endif // QUAD_H