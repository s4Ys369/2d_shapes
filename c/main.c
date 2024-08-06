#include <libdragon.h>
#include "point.h"
#include "render.h"
#include "shapes.h"

#include "rspq_constants.h"
#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
#include "profile_print.h"
#endif // RSPQ_PROFILE

// Global variables
surface_t disp;
int example, triCount, vertCount, currVerts, currTris, fillTris;
float stickX, stickY;
uint64_t bootTime, firstTime, secondTime, dispTime, jpTime, drawTime;
uint32_t screenWidth, screenHeight, frameCounter;

// Shape pointers
Shape* currShape;
Shape* circle;
Shape* quad;
Shape* fan;

Shape* curve;
Shape* curve2;
Point pointA;
Point pointB;
Point pointC;
Point pointD;
Point resetA;
Point resetB;
Point resetC;
Point resetD;
int resetCurve = 0;
size_t controlPoint = 0;
PointArray* bezierPoints;
PointArray* basePoints;
int bezierMode = 0;


// Local variables
Point currCenter;
Point screenCenter;
float currRadiusX = 0;
float currRadiusY = 0;
float currThickness = 0;
int currSegments = 0;
float currLOD = 0.0f;
float currAngle = 0.0f;
PointArray* previousPoints;
PointArray* currPoints; 

color_t currShapeColor;


// Texture test
static sprite_t *test_sprite;

int ramUsed = 0;

// Initialize libdragon
void setup() {

  debug_init_isviewer();
  debug_init_usblog();
    
  dfs_init(DFS_DEFAULT_LOCATION);

  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
  screenWidth = display_get_width();
  screenHeight = display_get_height();
  screenCenter = point_new(screenWidth/2,screenHeight/2);
  disp = surface_alloc(FMT_RGBA16, screenWidth, screenHeight);

  rdpq_init();
#ifdef DEBUG_RDPQ
  rdpq_debug_start();
#endif // DEBUG_RDPQ

#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
  profile_data.frame_count = 0;
  rspq_profile_start();
#endif // RSPQ_PROFILE

  joypad_init();

  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));


  // Initialize acummulators
  bootTime= 0;
  firstTime = 0;
  secondTime = 0;
  jpTime = 0;
  dispTime = 0;
  drawTime = 0;
  frameCounter = 0;
  example = 0;
  triCount = 0;
  vertCount = 0;
  currTris = 0;
  fillTris = 0;
  currVerts = 0;
  stickX = 0.0f;
  stickY = 0.0f;

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

  // Circle
  circle = (Shape*)malloc_uncached(sizeof(Shape));
  circle_init(circle, screenCenter, 20.0f, 0.05f, RED);

  // Quad as a strip
  quad = (Shape*)malloc_uncached(sizeof(Shape));
  strip_init(quad, screenCenter, 20.0f, 20.0f, 0.01f, 1, DARK_GREEN);

  // Fan has only scale, whereas fan2 has both X and Y scales
  fan = (Shape*)malloc_uncached(sizeof(Shape));
  fan2_init(fan, screenCenter, 20.0f, 20.0f, 5, BLUE);

  // Curves are treat as strips
  curve = (Shape*)malloc_uncached(sizeof(Shape));
  strip_init(curve, screenCenter, 20.0f, 20.0f, 2.0f, 10, RED);
  curve2 = (Shape*)malloc_uncached(sizeof(Shape));
  strip_init(curve2, screenCenter, 20.0f, 20.0f, 2.0f, 10, GREEN);

  // Texture test
  test_sprite = sprite_load("rom:/n64brew.sprite");
  rdpq_sprite_upload(TILE0, test_sprite, NULL);


  // Set up control points for Bezier examples
  pointA = point_new( ((float)(screenWidth/2) - 40.0f), ((float)(screenHeight/2) + 20.0f) );
  pointB = point_new( ((float)(screenWidth/2) - 20.0f), ((float)(screenHeight/2) - 40.0f) );
  pointC = point_new( ((float)(screenWidth/2) + 20.0f), ((float)(screenHeight/2) - 40.0f) );
  pointD = point_new( ((float)(screenWidth/2) + 40.0f), ((float)(screenHeight/2) + 20.0f) );

  resetA = pointA;
  resetB = pointB;
  resetC = pointC;
  resetD = pointD;

  Point points[] = {pointA, pointB, pointC, pointD, screenCenter};
  size_t numPoints = sizeof(points) / sizeof(points[0]);

  bezierPoints = (PointArray*)malloc_uncached(sizeof(PointArray));
  init_point_array_from_points(bezierPoints, points, numPoints);

  Point resets[] = {resetA, resetB, resetC, resetD, screenCenter};
  size_t numResets = sizeof(points) / sizeof(points[0]);

  basePoints = (PointArray*)malloc_uncached(sizeof(PointArray));
  init_point_array_from_points(basePoints, resets, numResets);

}

void circle_draw(){
  currShape = circle;

  // Get ellipse points and store them in currPoints
  render_get_ellipse_points(currShape->currPoints, currCenter, currRadiusX, currRadiusY, currSegments);

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

// Main rendering function
void draw() {
  
  switch (example) {
    case 0:
      circle_draw();
      break;
    case 1:
      currShape = quad;
      resolve(quad, stickX, stickY);
      currCenter = get_center(currShape);
      currRadiusX = get_scaleX(currShape);
      currRadiusY = get_scaleY(currShape);
      currThickness = get_thickness(currShape);
      currShapeColor = get_fill_color(currShape);
      set_segments(currShape, 1); // Always initializes as at least 3 for ellipse, but a quad has only one segment per draw
      currSegments = get_segments(currShape);
      set_render_color(currShapeColor);
      draw_line(
        currCenter.x-currRadiusX, currCenter.y-currRadiusY, 
        currCenter.x+currRadiusX, currCenter.y+currRadiusY, 
        currAngle,
        currThickness
      );
      break;
    case 2:
      currShape = fan;
      currCenter = get_center(currShape);
      currRadiusX = get_scaleX(currShape);
      currRadiusY = get_scaleY(currShape);
      currSegments = get_segments(currShape);
      currLOD = get_lod(currShape);

      render_get_ellipse_points(previousPoints, currCenter, currRadiusX, currRadiusY, currSegments);
      set_points(currShape, currPoints);

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
      break;
    case 3:
      currShape = curve;
      currCenter = get_center(currShape);
      currRadiusX = get_scaleX(currShape);
      currRadiusY = get_scaleY(currShape);
      currSegments = get_segments(currShape);
      if(currSegments > 100){
        currSegments = 5;
      }
      currThickness = get_thickness(currShape);
      if(currThickness > 10.0f){
        currThickness = 1.0f;
      }

      if(resetCurve == 0){
        render_move_point(bezierPoints, controlPoint, stickX*0.05f, -stickY*0.05f);
        render_rotate_point(bezierPoints, controlPoint, currCenter, currAngle*0.05f);
      } else {
        bezierPoints = basePoints;
      }

      if(controlPoint == bezierPoints->count - 1){
        resolve(curve, stickX,stickY);
        render_move_shape_points(bezierPoints, stickX*0.05f, -stickY*0.05f);
        render_rotate_shape_points(bezierPoints, currCenter, currAngle*0.05f);
      }

      //debugf("Total bezierPoints %d\n", bezierPoints.size());

      // Limit movement to inside screen with offset here because shape.resolve(x,y) doesn't apply to curves
      float offset = 5.0f;
      float width = display_get_width();
      float height = display_get_height();

      for( size_t i = 0; i < bezierPoints->count - 1; ++i) {
        if (bezierPoints->points[controlPoint].x < offset) {
          bezierPoints->points[controlPoint].x = offset;
        }
        if (bezierPoints->points[controlPoint].x > width - offset) {
          bezierPoints->points[controlPoint].x = width - offset;
        }
        if (bezierPoints->points[controlPoint].y < offset) {
          bezierPoints->points[controlPoint].y = offset;
        }
        if (bezierPoints->points[controlPoint].y > height - offset) {
          bezierPoints->points[controlPoint].y = height - offset;
        }
      }

      currShapeColor = get_fill_color(currShape);
      set_render_color(currShapeColor);
      draw_bezier_curve(
        &pointA, &pointB, &pointC, &pointD,
        currSegments,
        currAngle,
        currThickness
      );

      set_render_color(BLUE);
      draw_filled_beziers(
        &bezierPoints->points[0], &bezierPoints->points[1], &bezierPoints->points[2], &bezierPoints->points[3],
        &basePoints->points[0], &basePoints->points[1], &basePoints->points[2], &basePoints->points[3],
        currSegments
      );

      set_render_color(get_fill_color(curve2));
      draw_bezier_curve(
        &resetA, &resetB, &resetC, &resetD,
        currSegments,
        0.0f,
        currThickness
      );


      set_render_color(BLACK);
      for( size_t i = 0; i < bezierPoints->count; ++i){
        draw_circle(bezierPoints->points[i].x, bezierPoints->points[i].y, 2.0f, 2.0f, currAngle, 0.01f);
      }
      set_render_color(YELLOW);
      draw_circle(bezierPoints->points[controlPoint].x, bezierPoints->points[controlPoint].y, 1.5f, 1.5f, currAngle, 0.01f);

      set_points(currShape, bezierPoints);
      currPoints = get_points(currShape);

      
      pointA = currPoints->points[0];
      pointB = currPoints->points[1];
      pointC = currPoints->points[2];
      pointD = currPoints->points[3];
      

      //debugf(
      //  "A (%.2f,%.2f)\n"
      //  "B (%.2f,%.2f)\n"
      //  "C (%.2f,%.2f)\n"
      //  "D (%.2f,%.2f)\n", 
      //  pointA.x, pointA.y,
      //  pointB.x, pointB.y,
      //  pointC.x, pointC.y,
      //  pointD.x, pointD.y
      //);
      
      break;
  }

}

void reset_example() {
  currAngle = 0;
  if(currShape == circle){
    set_center(currShape, screenCenter);
    set_scaleX(currShape, 20.0f);
    set_lod(currShape, 0.05f);
  } else if(currShape == fan){
    set_center(currShape, screenCenter);
    set_scaleX(currShape, 20.0f);
    set_scaleY(currShape, 20.0f);
    set_lod(currShape, 0.05f);
    set_segments(currShape, 5);
    controlPoint = 0;
  } else if (currShape == curve) {
    set_center(currShape, screenCenter);
    set_scaleX(currShape, 20.0f);
    set_scaleY(currShape, 20.0f);
    set_thickness(currShape, 2.0f);
    set_segments(currShape, 10);
    controlPoint = 0;
    free_point_array(currPoints);
    resetCurve = 1;
    pointA = resetA;
    pointB = resetB;
    pointC = resetC;
    pointD = resetD;
  } else { // Quad
    set_center(currShape, screenCenter);
    set_scaleX(currShape, 20.0f);
    set_scaleY(currShape, 20.0f);
    set_thickness(currShape, 0.01f);
  }
}

void switch_example() {
  if (++example > 1) {
    example = 0;
    reset_example();
  }
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
  if(currShape != (Shape*)circle){
    if(get_segments(currShape) < 20){
      set_segments(currShape, get_segments(currShape) + 1);
    } else {
      set_segments(currShape, 5);
    }
  } else {
    increase_lod(currShape);
  }
}

void decrease_segments(Shape *currShape) {
  if(currShape != (Shape*)circle){
    if(get_segments(currShape) > 5){
      set_segments(currShape, get_segments(currShape) - 1);
    } else {
      set_segments(currShape, 20);
    }
  } else {
    decrease_lod(currShape);
  }
}

void cycle_control_point() {
  if(currShape != (Shape*)curve){
    currPoints = get_points(currShape);
    if(controlPoint < currPoints->count){
      controlPoint++;
    } else {
      controlPoint = 0;
    }
  } else {
    if(controlPoint < bezierPoints->count - 1){
      controlPoint++;
    } else {
      controlPoint = 0;
    }
  }
}

// Main function with rendering loop
int main() {
  setup();
  bootTime = get_ticks_ms();

  for (;;) {

    firstTime = get_ticks_ms();// set loop time

    rdpq_attach(display_get(), &disp);
    rdpq_clear(GREY);
    rdpq_clear_z(0xFFFC);

    rdpq_sync_pipe();
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);

    dispTime = get_ticks_ms() - firstTime; // set display time
    secondTime = get_ticks_ms();

    joypad_poll();
    joypad_inputs_t input = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t keys = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    joypad_buttons_t keysDown = joypad_get_buttons_held(JOYPAD_PORT_1);

    // RAM used from RDPQ Demo
    struct mallinfo mem_info = mallinfo();
    ramUsed = mem_info.uordblks - (size_t) (((display_get_width() * display_get_height()) * 2) - ((unsigned int) HEAP_START_ADDR - 0x80000000) - 0x10000);

    stickX = (float)input.stick_x;
    stickY = (float)input.stick_y;

    jpTime = get_ticks_ms() - secondTime; // set input time

    if (keys.l) { // CHANGE: L for console
      switch_example(); 
    }

    if (keys.start) {
      reset_example();
    }


//=========== ~ UPDATE ~ ==============//

    draw();

    // Shape update
    currCenter = get_center(currShape);
    currRadiusX = get_scaleX(currShape);
    currSegments = get_segments(currShape);
    currLOD = get_lod(currShape);

    if(currShape == circle){
      set_segments(currShape, triCount); // For the ellipse (ie fan) segments and triangles are essentially the same
      if(currLOD < ((float)currSegments*0.01f)){
        currLOD = ((float)currSegments*0.01f);
      }
    } else if(currShape == quad){
      set_segments(currShape, triCount/2); // 1 segment per 2 triangles for the quad
      currRadiusY = get_scaleY(currShape);
      currThickness = get_thickness(currShape);
    } else if(currShape == curve){
      if(currSegments > 30){
        currSegments = 5;
      }
    } else {
      if(currLOD < ((float)currSegments*0.01f)){
        currLOD = ((float)currSegments*0.01f);
      }
    }

    currShapeColor = get_fill_color(currShape);
    render_get_ellipse_points(currShape->currPoints, currCenter, currRadiusX, currRadiusY, currSegments);
    set_points(currShape, previousPoints);

//=========== ~ CONTROLS ~ ==============//

    // Add rotation
    float rotation = (float)(M_PI)/18.0f; // ~10 degrees
    float rotationDegrees = currAngle * radiansToDegrees;

    if(fabsf(rotationDegrees) > 360.0f) {
      currAngle = 0;
      rotationDegrees = 0;
    }

    if(keysDown.a){
      currAngle += rotation;
    } else if (keysDown.b) {
      currAngle -= rotation;
    }

    if(currShape != curve) {
      // Adjust single scale shape
      if(keysDown.r){
        increase_scale(currShape);
      }
      if(keysDown.z){ // CHANGE: Z for console
        decrease_scale(currShape);
      }
    } else {
      if(keys.r){
        decrease_segments(currShape);
      }
      if(keys.z){ // CHANGE: Z for console
        increase_segments(currShape);
      }
    }

    if(currShape == circle){
      if(keysDown.c_left){
        increase_lod(currShape);
      }
      if(keysDown.c_down){
        decrease_lod(currShape);
      }
    } else if(currShape != curve) {
      // Fine tunes individual scales
      if(keysDown.c_up){
        increase_y_scale(currShape);
      }
      if(keysDown.c_down){
        decrease_y_scale(currShape);
      }
      if(keysDown.c_right){
        increase_x_scale(currShape);
      }  
      if(keysDown.c_left){
        decrease_x_scale(currShape);
      }

      // Specific to fan example
      if(currShape == fan) {
        if(keys.d_up){
          increase_segments(currShape);
        }
        if(keys.d_down){
          decrease_segments(currShape);
        }
        if(keys.d_right){
          cycle_control_point();
        }  
        if(keys.d_left){
          cycle_control_point();
        }
      }

    } else {
      if(keys.c_down){
        cycle_control_point();
      }
      if(keys.c_left){
        cycle_control_point();
      }
    }


//=========== ~ UI ~ =============//

    if(frameCounter > 59){
      drawTime = ((get_ticks_ms() - secondTime) + dispTime + jpTime); // CPU time after draw and transform
      frameCounter = 0;
    }



    if(currShape == circle){

      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 20, 
        "Circle\n\n"
        "Diameter: %.0fpx\n"
        "Rotation: %.0f\n"
        "Verts: %u\n"
        "LOD: %.2f\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "CPU Time: %lldms\n\n"
        "Stick to Move\n"
        "R/Z: Scale\n"
        "CL/CD: LOD\n"
        "A/B: Rotate\n"
        "Start: Reset Example\n"
        "L: Switch Example\n"
        "RAM %dKB/%dKB",
        currRadiusX*2, // For a ellipse, both scale values are the same and used to change the diameter of the polygon
        rotationDegrees,
        vertCount+1, // All triangles in the fan use the center vertex and previous vertex, so only accumulate 1 per draw, then add center here
        currLOD,
        triCount,
        display_get_fps(),
        drawTime ,
        (ramUsed / 1024), (get_memory_size() / 1024)
      );
    } else if (currShape == quad) {

      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 20,
        "Quad\n\n"
        "Width: %.0fpx\n"
        "Height: %.0fpx\n"
        "Rotation: %.0f\n"
        "Verts: %u\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "CPU Time: %lldms\n\n"
        "Stick to Move\n"
        "R/Z: Scale\n"
        "CL/CR: X Scale\n"
        "CU/CD: Y Scale\n"
        "A/B: Rotate\n"
        "Start: Reset Example\n"
        "L: Switch Example\n"
        "RAM %dKB/%dKB",
        currRadiusX*2,
        currRadiusY*2,
        rotationDegrees,
        vertCount, // Always 4 vertices per quad
        triCount, // Always 2 triangles per quad
        display_get_fps(),
        drawTime,
        (ramUsed / 1024), (get_memory_size() / 1024)
      );
    } else if (currShape == curve) {

      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 20,
        "Bezier Curves with Fill\n\n"
        "Control: %d/%d\n"
        "Rotation: %.0f\n"
        "Segments: %u\n"
        "Fill Tris: %u\n"
        "Curve Tris: %u\n"
        "FPS: %.2f\n"
        "CPU Time: %lldms\n\n"
        "Stick to Move\n"
        "Z/R: Cycle Segments\n"
        "CL/CD: Cycle Control\n"
        "A/B: Rotate\n"
        "Start: Reset Example\n"
        "L: Switch Example\n"
        "RAM %dKB/%dKB",
        controlPoint+1, // Point being transformed, where the last of the current Points is the center of the fan
        5,
        rotationDegrees,
        //currLOD,
        currSegments,
        //2 * (currSegments + 1),
        fillTris,
        currTris,
        display_get_fps(),
        drawTime,
        (ramUsed / 1024), (get_memory_size() / 1024)
      );
    } else {

      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 20,
        "Triangle Fan\n"
        "Scale: (%.0fpx,%.0fpx)\n"
        "Rotation: %.0f\n"
        "Segments: %u\n"
        "Verts: %d/%d\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "CPU Time: %lldms\n"
        "Stick to Move\n"
        "R/Z: Scale\n"
        "CL/CR: X Scale\n"
        "CU/CD: Y Scale\n"
        "DU/DD: Scale Segments\n"
        "DL/DR: Cycle Verts\n"
        "A/B: Rotate\n"
        "Start: Reset Example\n"
        "L: Switch Example\n"
        "RAM %dKB/%dKB",
        currRadiusX*2,
        currRadiusY*2,
        rotationDegrees,
        currSegments,
        controlPoint+1, // Point being transformed, where the last of the current Points is the center of the fan
        (currPoints->count+1), // Always triangles + center
        currSegments, // Always verts - center
        display_get_fps(),
        drawTime,
        (ramUsed / 1024), (get_memory_size() / 1024)
      );
    }

    // Reset acummulators
    triCount = 0;
    vertCount = 0;
    currTris = 0;
    fillTris = 0;
    currVerts = 0;
    firstTime = 0;
    secondTime = 0;
    jpTime = 0;
    dispTime = 0;
    resetCurve = 0;

    frameCounter++;
    
    rdpq_detach_show();

#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
    rspq_profile_next_frame();

  // Every second we profile the RSPQ
    if(frameCounter > 59){
      for (size_t i = 0; i < RSPQ_PROFILE_SLOT_COUNT; i++) {
        profile_data.slots[i].sample_count = 1000 + i * 100;
        profile_data.slots[i].total_ticks = 2000000 + i * 200000;
      }
      debug_print_profile_data();// prints all profiler data to console, use sparingly
      rspq_profile_reset();    
    }

    rspq_profile_get_data(&profile_data);
#endif // RSPQ_PROFILE

  }

  //=========== ~ CLEAN UP ~ =============//
  destroy(currShape);
  destroy(circle);
  destroy(quad);
  destroy(fan);
  destroy(curve);
  destroy(curve2);
  free_point_array(bezierPoints);
  free_point_array(basePoints);
  free_point_array(currPoints);
  return 0;
}
