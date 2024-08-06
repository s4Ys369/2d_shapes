#ifndef CONTROL_H
#define CONTROL_H

#include <libdragon.h>
#include "globals.h"
#include "../render.h"
#include "../shapes.h"

Shape* currShape;
Point currCenter;
float currRadiusX;
float currRadiusY;
float currThickness;
int currSegments;
float currLOD ;
float currAngle ;
PointArray* previousPoints;
PointArray* currPoints; 
color_t currShapeColor;

void shape_control_init() {
// Allocate a dummy/control shape
  currShape = (Shape*)malloc_uncached(sizeof(Shape));
  shape_init(currShape);
  init_point_array(currShape->currPoints);
  currPoints = (PointArray*)malloc_uncached(sizeof(PointArray));
  init_point_array_from_points(currPoints, currShape->currPoints->points, currShape->currPoints->count);
  currPoints = currShape->currPoints;
  currShapeColor = get_fill_color(currShape);
  currCenter = get_center(currShape);
  previousPoints = (PointArray*)malloc_uncached(sizeof(PointArray));
  init_point_array(previousPoints);
}

void increase_scale(Shape *currShape) {
  if(get_scaleX(currShape) < screenWidth/2){
    set_scaleX(currShape, get_scaleX(currShape) + 1.0f);
  } else {
    set_scaleX(currShape, 1.0f);
  }
}

void decrease_scale(Shape *currShape) {
  if(get_scaleX(currShape) > 1.0f){
    set_scaleX(currShape, get_scaleX(currShape) - 1.0f);
  } else {
    set_scaleX(currShape, screenWidth/2);
  }
}

void increase_x_scale(Shape *currShape) {
  float currentScaleX = get_scaleX(currShape);
  if(currentScaleX < screenWidth){
    set_scaleX(currShape, currentScaleX + 0.1f);
  } else {
    set_scaleX(currShape, 1.0f);
  }
}

void decrease_x_scale(Shape *currShape) {
  float currentScaleX = get_scaleX(currShape);
  if(currentScaleX > 1.1f){
    set_scaleX(currShape, currentScaleX - 0.1f);
  } else {
    set_scaleX(currShape, screenWidth);
  }
}

void increase_y_scale(Shape *currShape) {
  float currentScaleY = get_scaleY(currShape);
  if(currentScaleY < screenHeight){
    set_scaleY(currShape, currentScaleY + 0.1f);
  } else {
    set_scaleY(currShape, 1.0f);
  }
}

void decrease_y_scale(Shape *currShape) {
  float currentScaleY = get_scaleY(currShape);
  if(currentScaleY > 1.1f){
    set_scaleY(currShape, currentScaleY - 0.1f);
  } else {
    set_scaleY(currShape, screenHeight);
  }
}

void increase_lod(Shape *currShape) {
  if(get_lod(currShape) < 2.0f){
    set_lod(currShape, get_lod(currShape) + 0.05f);
  } else {
    set_lod(currShape, 0.05f);
  }
}

void decrease_lod(Shape *currShape) {
  if(get_lod(currShape) >= 0.1f){
    set_lod(currShape, get_lod(currShape) - 0.05f);
  } else {
    set_lod(currShape, 2.0f);
  }
}

void increase_thickness(Shape *currShape) {
  if(get_thickness(currShape) < 10.0f){
    set_thickness(currShape, get_thickness(currShape) + 0.01f);
  } else {
    set_thickness(currShape, 0.01f);
  }
}

void decrease_thickness(Shape *currShape) {
  if(get_thickness(currShape) >= 0.02f){
    set_thickness(currShape, get_thickness(currShape) - 0.01f);
  } else {
    set_thickness(currShape, 10.0f);
  }
}

void increase_segments(Shape *currShape) {
  if(get_segments(currShape) < 20){
    set_segments(currShape, get_segments(currShape) + 1);
  } else {
    set_segments(currShape, 5);
  }
}

void decrease_segments(Shape *currShape) {
  if(get_segments(currShape) > 5){
    set_segments(currShape, get_segments(currShape) - 1);
  } else {
    set_segments(currShape, 20);
  }
}

void cycle_control_point() {
  currPoints = get_points(currShape);
  if(controlPoint < currPoints->count){
    controlPoint++;
  } else {
    controlPoint = 0;
  }
}

void cycle_bezier_points(){
  if(controlPoint < 4){
    controlPoint++;
  } else {
    controlPoint = 0;
  }
}

#endif // CONTROL_H