#ifndef BEZIER_H
#define BEZIER_H

#include <libdragon.h>
#include "control.h"

Shape* curve;
Shape* curve2;
int resetCurve;
PointArray* bezierPoints;
PointArray* basePoints;

void create_bezier(){
  // Curves are treat as strips
  curve = (Shape*)malloc_uncached(sizeof(Shape));
  strip_init(curve, screenCenter, 20.0f, 20.0f, 2.0f, 10, RED);
  curve2 = (Shape*)malloc_uncached(sizeof(Shape));
  strip_init(curve2, screenCenter, 20.0f, 20.0f, 2.0f, 10, GREEN);

  // Set up control points for transformable curve (curve)
  Point points[] = {
    point_new(((float)(screenWidth/2) - 40.0f), ((float)(screenHeight/2) + 20.0f)),
    point_new(((float)(screenWidth/2) - 20.0f), ((float)(screenHeight/2) - 40.0f)),
    point_new(((float)(screenWidth/2) + 20.0f), ((float)(screenHeight/2) - 40.0f)),
    point_new(((float)(screenWidth/2) + 40.0f), ((float)(screenHeight/2) + 20.0f)),
    screenCenter
  };
  size_t numPoints = sizeof(points) / sizeof(points[0]);

  bezierPoints = (PointArray*)malloc_uncached(sizeof(PointArray));
  if (!bezierPoints) {
    debugf("Failed to allocate bezierPoints\n");
    return;
  }
  init_point_array_from_points(bezierPoints, points, numPoints);

  // Set up control points for static curve (curve2)
  Point resets[] = {
    points[0], // Reset points should be identical to initial points
    points[1],
    points[2],
    points[3],
    screenCenter
  };
  size_t numResets = sizeof(resets) / sizeof(resets[0]);

  basePoints = (PointArray*)malloc_uncached(sizeof(PointArray));
  if (!basePoints) {
    debugf("Failed to allocate basePoints\n");
    free(bezierPoints->points); // Clean up previously allocated memory
    free(bezierPoints);
    return;
  }
  init_point_array_from_points(basePoints, resets, numResets);

  resetCurve = 0;
}

void bezier_draw(){

  // Update current shape properties
  currShape = curve;
  currCenter = get_center(currShape);
  currRadiusX = get_scaleX(currShape);
  currRadiusY = get_scaleY(currShape);
  currSegments = get_segments(currShape);

  // Arbitrary values for example
  if(currSegments > 100){
    currSegments = 5; // < 5 segments starts to stop looking like a curve
  }

  // Thickness and LOD are same concerning strips, names are just for easy of use
  currThickness = get_thickness(currShape);
  if(currThickness > 10.0f){
    currThickness = 1.0f;
  }


  if(resetCurve == 0){
    // FIXME: render_move and render_rotate need to be adjusted if the damping needs to be 0.05f
    render_move_point(bezierPoints, controlPoint, stickX*0.05f, -stickY*0.05f);
    render_rotate_point(bezierPoints, controlPoint, currCenter, currAngle*0.05f);
  } else {
    // Set all the points of the transformed curve to the static one
    for (size_t i = 0; i < basePoints->count; ++i) {
      bezierPoints->points[i] = basePoints->points[i];
    }
    resetCurve = 0;
  }

  if(controlPoint == bezierPoints->count - 1){
    // FIXME: render_move and render_rotate need to be adjusted if the damping needs to be 0.05f
    render_move_shape_points(bezierPoints, stickX*0.05f, -stickY*0.05f);
    render_rotate_shape_points(bezierPoints, currCenter, currAngle*0.05f);
  }

  // Limit movement to inside screen with offset here because shape.resolve(x,y) doesn't apply to curves
  float offset = 5.0f;

  for( size_t i = 0; i < bezierPoints->count - 1; ++i) {
    if (bezierPoints->points[controlPoint].x < offset) {
      bezierPoints->points[controlPoint].x = offset;
    }
    if (bezierPoints->points[controlPoint].x > screenWidth - offset) {
      bezierPoints->points[controlPoint].x = screenWidth - offset;
    }
    if (bezierPoints->points[controlPoint].y < offset) {
      bezierPoints->points[controlPoint].y = offset;
    }
    if (bezierPoints->points[controlPoint].y > screenHeight - offset) {
      bezierPoints->points[controlPoint].y = screenHeight - offset;
    }
  }

  // Transformable curve
  currShapeColor = get_fill_color(currShape);
  set_render_color(currShapeColor);
  draw_bezier_curve(
    &bezierPoints->points[0], &bezierPoints->points[1], &bezierPoints->points[2], &bezierPoints->points[3],
    currSegments,
    currAngle,
    currThickness
  );

  // Fill strips
  set_render_color(BLUE);
  draw_filled_beziers(
    &bezierPoints->points[0], &bezierPoints->points[1], &bezierPoints->points[2], &bezierPoints->points[3],
    &basePoints->points[0], &basePoints->points[1], &basePoints->points[2], &basePoints->points[3],
    currSegments
  );

  // Static curve
  set_render_color(get_fill_color(curve2));
  draw_bezier_curve(
    &basePoints->points[0], &basePoints->points[1], &basePoints->points[2], &basePoints->points[3],
    currSegments,
    0.0f,
    currThickness
  );

  // Control Points
  set_render_color(BLACK);
  for( size_t i = 0; i < bezierPoints->count; ++i){
    draw_circle(bezierPoints->points[i].x, bezierPoints->points[i].y, 2.0f, 2.0f, currAngle, 0.01f);
  }

  // Selected Control Point
  set_render_color(YELLOW);
  draw_circle(bezierPoints->points[controlPoint].x, bezierPoints->points[controlPoint].y, 1.5f, 1.5f, currAngle, 0.01f);

}



#endif // BEZIER_H