#ifndef FAN_H
#define FAN_H

#include <libdragon.h>
#include "control.h"

Shape* fan;

void create_fan(){
// `fan` has only scale, whereas `fan2` has both X and Y scales
  fan = (Shape*)malloc_uncached(sizeof(Shape));
  fan2_init(fan, screenCenter, 20.0f, 20.0f, 3, BLUE);
}

void fan_draw(){
  currShape = fan;

  // Get ellipse points and store them in currPoints
  render_get_ellipse_points(currPoints, currCenter, currRadiusX, currRadiusY, currSegments);

  // Update current shape properties
  currCenter = get_center(currShape);
  currRadiusX = get_scaleX(currShape);
  currRadiusY = get_scaleY(currShape);
  currSegments = get_segments(currShape);
  currLOD = get_lod(currShape);
  currShapeColor = get_fill_color(currShape);

  // Start example with center selected
  controlPoint = currPoints->count;

  // Update rotation then position based on joypad input
  render_rotate_point(currPoints, controlPoint, currCenter, currAngle);
  render_move_point(currPoints, controlPoint, stickX, -stickY);
  if(controlPoint == currPoints->count){
    render_rotate_shape_points(currPoints, currCenter, currAngle);
    resolve(currShape, stickX, stickY);
  }

  // Should be as easy as generate verts, set color, draw    
  //set_render_color(currShapeColor);
  draw_rdp_fan(currPoints, currCenter);

  // Draw selected control point
  if ( controlPoint < currPoints->count){
    set_render_color(BLACK);
    draw_circle(currPoints->points[controlPoint].x, currPoints->points[controlPoint].y, 3.0f, 3.0f, 0.0f, 0.05f);
    set_render_color(YELLOW);
    draw_circle(currPoints->points[controlPoint].x, currPoints->points[controlPoint].y, 2.0f, 2.0f, 0.0f, 0.05f);
  } else {
    set_render_color(BLACK);
    draw_circle(currCenter.x, currCenter.y, 3.0f, 3.0f, 0.0f, 0.05f);
    set_render_color(YELLOW);
    draw_circle(currCenter.x, currCenter.y, 2.0f, 2.0f, 0.0f, 0.05f);
  }

  /*
    Since points are re-generated every frame, clear after drawing.
    Using set_points like from the circle example will
    clear the points of the Shape if not NULL.
  */
  free(currPoints->points);
}


#endif // FAN_H