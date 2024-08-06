#ifndef FAN_H
#define FAN_H

#include <libdragon.h>
#include "control.h"

Shape* fan;

void create_fan(){
// Fan has only scale, whereas fan2 has both X and Y scales
  fan = (Shape*)malloc_uncached(sizeof(Shape));
  fan2_init(fan, screenCenter, 20.0f, 20.0f, 5, BLUE);
}

void fan_draw(){
  currShape = fan;

  render_get_ellipse_points(currPoints, currCenter, currRadiusX, currRadiusY, currSegments);
  currCenter = get_center(currShape);
  currRadiusX = get_scaleX(currShape);
  currRadiusY = get_scaleY(currShape);
  currSegments = get_segments(currShape);
  currLOD = get_lod(currShape);
  currShapeColor = get_fill_color(currShape);

  render_move_point(currPoints, controlPoint, stickX, -stickY);
  render_rotate_point(currPoints, controlPoint, currCenter, currAngle);
  if(controlPoint == currPoints->count){
    render_move_shape_points(currPoints, stickX, -stickY);
    render_rotate_shape_points(currPoints, currCenter, currAngle);
  }
      
  set_render_color(currShapeColor);
  draw_fan(currPoints);

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

  free(currPoints->points);
}


#endif // FAN_H