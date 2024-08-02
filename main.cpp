#include <libdragon.h>
#include "Point.h"
#include "Render.h"
#include "Shape.h"
#include "Utils.h"

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
Shape* ellipse;
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
std::size_t controlPoint = 0;
std::vector<Point> bezierPoints;
std::vector<Point> basePoints;
int bezierMode = 0;


// Local variables
Point currCenter = 0;
float currRadiusX = 0;
float currRadiusY = 0;
float currThickness = 0;
int currSegments = 0;
float currLOD = 0.0f;
float currAngle = 0.0f;
std::vector<Point> currPoints; 

color_t currShapeColor = BLACK;


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

  // Initialize shapes
  ellipse = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 0.05f, RED);
  quad = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 20.0f, 0.01f, 1, DARK_GREEN);
  fan = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 20.0f, 5, BLUE);
  curve = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 20.0f, 2.0f, 10, RED);
  curve2 = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 20.0f, 2.0f, 10, GREEN);

  // Texture test
  test_sprite = sprite_load("rom:/n64brew.sprite");
  rdpq_sprite_upload(TILE0, test_sprite, NULL);


  // Set up control points for Bezier examples
  pointA = Point( ((float)(screenWidth/2) - 40.0f), ((float)(screenHeight/2) + 20.0f) );
  pointB = Point( ((float)(screenWidth/2) - 20.0f), ((float)(screenHeight/2) - 40.0f) );
  pointC = Point( ((float)(screenWidth/2) + 20.0f), ((float)(screenHeight/2) - 40.0f) );
  pointD = Point( ((float)(screenWidth/2) + 40.0f), ((float)(screenHeight/2) + 20.0f) );

  resetA = pointA;
  resetB = pointB;
  resetC = pointC;
  resetD = pointD;

  bezierPoints.reserve(5);
  bezierPoints = {pointA, pointB, pointC, pointD, Point(screenWidth/2,screenHeight/2)};
  basePoints.reserve(5);
  basePoints = {resetA, resetB, resetC, resetD, Point(screenWidth/2,screenHeight/2)};

  

}

// Main rendering function
Render renderer;
void draw() {
  
  switch (example) {
    case 0:
      currShape = ellipse;
      ellipse->resolve(stickX, stickY);
      currCenter = currShape->get_center();
      currRadiusX = currShape->get_scaleX();
      currRadiusY = currShape->get_scaleY();
      currSegments = currShape->get_segments();
      currLOD = currShape->get_lod();
      if(currLOD < ((float)currSegments*0.01f)){
        currLOD = ((float)currSegments*0.01f);
      }
      currShapeColor = currShape->get_shape_fill_color();
      renderer.set_fill_color(currShapeColor);
      currShape->set_points(renderer.get_ellipse_points(currCenter, currRadiusX, currRadiusY, currSegments));
      renderer.draw_ellipse(currCenter.x, currCenter.y, currRadiusX, currRadiusY, currAngle, currLOD);
      currPoints.clear();
      currPoints = currShape->get_points();
      break;
    case 1:
      currShape = quad;
      quad->resolve(stickX, stickY);
      currCenter = currShape->get_center();
      currRadiusX = currShape->get_scaleX();
      currRadiusY = currShape->get_scaleY();
      currThickness = currShape->get_lod();
      currShapeColor = currShape->get_shape_fill_color();
      currShape->set_segments(1); // Always initializes as at least 3 for ellipse, but a quad has only one segment per draw
      renderer.set_fill_color(currShapeColor);
      renderer.draw_line(
        currCenter.x-currRadiusX, currCenter.y-currRadiusY, 
        currCenter.x+currRadiusX, currCenter.y+currRadiusY, 
        currAngle,
        currThickness
      );
      break;
    case 2:
      currShape = fan;

      currCenter = currShape->get_center();
      currRadiusX = currShape->get_scaleX();
      currRadiusY = currShape->get_scaleY();
      currSegments = currShape->get_segments();
      currLOD = currShape->get_lod();

      currShape->set_points(renderer.get_ellipse_points(currCenter, currRadiusX, currRadiusY, currSegments));
      currPoints.clear();
      currPoints = currShape->get_points();

      renderer.move_point(currPoints, controlPoint, stickX, -stickY);
      renderer.rotate_point(currPoints, controlPoint, currCenter, currAngle);
      if(controlPoint == currPoints.size()){
        renderer.move_shape_points(currPoints, stickX, -stickY);
        renderer.rotate_shape_points(currPoints, currCenter, currAngle);
      }
      
      renderer.set_fill_color(currShapeColor);
      renderer.draw_fan(currPoints);

      if ( controlPoint < currPoints.size()){
        renderer.set_fill_color(BLACK);
        renderer.draw_ellipse(currPoints[controlPoint].x, currPoints[controlPoint].y, 3.0f, 3.0f, 0.0f, 0.05f);
        renderer.set_fill_color(YELLOW);
        renderer.draw_ellipse(currPoints[controlPoint].x, currPoints[controlPoint].y, 2.0f, 2.0f, 0.0f, 0.05f);
      } else {
        renderer.set_fill_color(YELLOW);
        renderer.draw_ellipse(currCenter.x, currCenter.y, 3.0f, 3.0f, 0.0f, 0.05f);
        renderer.set_fill_color(LIGHT_GREY);
        renderer.draw_ellipse(currCenter.x, currCenter.y, 2.0f, 2.0f, 0.0f, 0.05f);
      }
      break;
    case 3:
      currShape = curve;
      currCenter = currShape->get_center();
      currRadiusX = currShape->get_scaleX();
      currRadiusY = currShape->get_scaleY();
      currSegments = currShape->get_segments();
      if(currSegments > 100){
        currSegments = 5;
      }
      currThickness = currShape->get_thickness();
      if(currThickness > 10.0f){
        currThickness = 1.0f;
      }

      if(resetCurve == 0){
        renderer.move_point(bezierPoints, controlPoint, stickX*0.05f, -stickY*0.05f);
        renderer.rotate_point(bezierPoints, controlPoint, currCenter, currAngle*0.05f);
      } else {
        bezierPoints = {resetA, resetB, resetC, resetD, Point(screenWidth/2,screenHeight/2)};
      }

      if(controlPoint == bezierPoints.size() - 1){
        currShape->resolve(stickX,stickY);
        renderer.move_shape_points(bezierPoints, stickX*0.05f, -stickY*0.05f);
        renderer.rotate_shape_points(bezierPoints, currCenter, currAngle*0.05f);
      }

      //debugf("Total bezierPoints %d\n", bezierPoints.size());

      // Limit movement to inside screen with offset here because shape.resolve(x,y) doesn't apply to curves
      float offset = 5.0f;
      float width = display_get_width();
      float height = display_get_height();

      for( size_t i = 0; i < bezierPoints.size() - 1; ++i) {
        if (bezierPoints[controlPoint].x < offset) {
          bezierPoints[controlPoint].x = offset;
        }
        if (bezierPoints[controlPoint].x > width - offset) {
          bezierPoints[controlPoint].x = width - offset;
        }
        if (bezierPoints[controlPoint].y < offset) {
          bezierPoints[controlPoint].y = offset;
        }
        if (bezierPoints[controlPoint].y > height - offset) {
          bezierPoints[controlPoint].y = height - offset;
        }
      }

      currShapeColor = currShape->get_shape_fill_color();
      renderer.set_fill_color(currShapeColor);
      renderer.draw_bezier_curve(
        pointA, pointB, pointC, pointD,
        currSegments,
        currAngle,
        currThickness
      );

      renderer.set_fill_color(BLUE);
      renderer.draw_filled_beziers(
        bezierPoints[0], bezierPoints[1], bezierPoints[2], bezierPoints[3],
        basePoints[0], basePoints[1], basePoints[2], basePoints[3],
        currSegments
      );

      renderer.set_fill_color(curve2->get_shape_fill_color());
      renderer.draw_bezier_curve(
        resetA, resetB, resetC, resetD,
        currSegments,
        0.0f,
        currThickness
      );


      renderer.set_fill_color(BLACK);
      for( size_t i = 0; i < bezierPoints.size(); ++i){
        renderer.draw_ellipse(bezierPoints[i].x, bezierPoints[i].y, 2.0f, 2.0f, currAngle, 0.01f);
      }
      renderer.set_fill_color(YELLOW);
      renderer.draw_ellipse(bezierPoints[controlPoint].x, bezierPoints[controlPoint].y, 1.5f, 1.5f, currAngle, 0.01f);

      currShape->set_points(bezierPoints);
      currPoints.clear();
      currPoints = currShape->get_points();

      
      pointA = currPoints[0];
      pointB = currPoints[1];
      pointC = currPoints[2];
      pointD = currPoints[3];
      

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
  if(currShape == ellipse){
    currShape->set_center(Point(screenWidth/2,screenHeight/2));
    currShape->set_scaleX(20.0f);
    currShape->set_lod(0.05f);
  } else if(currShape == fan){
    currShape->set_center(Point(screenWidth/2,screenHeight/2));
    currShape->set_scaleX(20.0f);
    currShape->set_scaleY(20.0f);
    currShape->set_lod(0.05f);
    currShape->set_segments(5);
    controlPoint = 0;
  } else if (currShape == curve) {
    currShape->set_center(Point(screenWidth/2,screenHeight/2));
    currShape->set_scaleX(20.0f);
    currShape->set_scaleY(20.0f);
    currShape->set_lod(2.0f);
    currShape->set_segments(10);
    controlPoint = 0;
    currPoints.clear();
    resetCurve = 1;
    pointA = resetA;
    pointB = resetB;
    pointC = resetC;
    pointD = resetD;
  } else {
    currShape->set_center(Point(screenWidth/2,screenHeight/2));
    currShape->set_scaleX(20.0f);
    currShape->set_scaleY(20.0f);
    currShape->set_lod(0.01f);
  }
}

void switch_example() {
  if (++example > 3) {
    example = 0;
    reset_example();
  }
}

void increase_scale(Shape *currShape) {
  if(currShape->get_scaleX() < (display_get_height()/2) && currShape->get_scaleX() < (display_get_width()/2)){
    currShape->set_scaleX(currShape->get_scaleX() + 1.0f);
    currShape->set_scaleY(currShape->get_scaleY() + 1.0f);
  } else {
    currShape->set_scaleX(1.0f);
    currShape->set_scaleY(1.0f);
  }
}

void decrease_scale(Shape *currShape) {
  if(currShape->get_scaleX() > 1.0f && currShape->get_scaleY() > 1.0f){
    currShape->set_scaleX(currShape->get_scaleX() - 1.0f);
    currShape->set_scaleY(currShape->get_scaleY() - 1.0f);
  } else {
    currShape->set_scaleX((display_get_height()/2));
    currShape->set_scaleY((display_get_width()/2));
  }
}

void increase_x_scale(Shape *currShape) {
  float currentScaleX = currShape->get_scaleX();
  if(currentScaleX < (display_get_height()/2)){
    currShape->set_scaleX(currentScaleX + 0.1f);
  } else {
    currShape->set_scaleX(1.0f);
  }
}

void decrease_x_scale(Shape *currShape) {
  float currentScaleX = currShape->get_scaleX();
  if(currentScaleX > 1.1f){
    currShape->set_scaleX(currentScaleX - 0.1f);
  } else {
    currShape->set_scaleX((display_get_height()/2));
  }
}

void increase_y_scale(Shape *currShape) {
  float currentScaleY = currShape->get_scaleY();
  if(currentScaleY < (display_get_width()/2)){
    currShape->set_scaleY(currentScaleY + 0.1f);
  } else {
    currShape->set_scaleY(1.0f);
  }
}

void decrease_y_scale(Shape *currShape) {
  float currentScaleY = currShape->get_scaleY();
  if(currentScaleY > 1.1f){
    currShape->set_scaleY(currentScaleY - 1.0f);
  } else {
    currShape->set_scaleY((display_get_width()/2));
  }
}

void increase_lod(Shape *currShape) {
  if(currShape->get_lod() < 2.0f){
    currShape->set_lod(currShape->get_lod() + 0.05f);
  } else {
    currShape->set_lod(0.05f);
  }
}

void decrease_lod(Shape *currShape) {
  if(currShape->get_lod() >= 0.1f){
    currShape->set_lod(currShape->get_lod() - 0.05f);
  } else {
    currShape->set_lod(2.0f);
  }
}

void increase_thickness(Shape *currShape) {
  if(currShape->get_lod() < 10.0f){
    currShape->set_lod(currShape->get_lod() + 0.01f);
  } else {
    currShape->set_lod(0.01f);
  }
}

void decrease_thickness(Shape *currShape) {
  if(currShape->get_lod() >= 0.02f){
    currShape->set_lod(currShape->get_lod() - 0.01f);
  } else {
    currShape->set_lod(10.0f);
  }
}

void increase_segments(Shape *currShape) {
  if(currShape != ellipse){
    if(currShape->get_segments() < 20){
      currShape->set_segments(currShape->get_segments() + 1);
    } else {
      currShape->set_segments(5);
    }
  } else {
    increase_lod(currShape);
  }
}

void decrease_segments(Shape *currShape) {
  if(currShape != ellipse){
    if(currShape->get_segments() > 5){
      currShape->set_segments(currShape->get_segments() - 1);
    } else {
      currShape->set_segments(20);
    }
  } else {
    decrease_lod(currShape);
  }
}

void cycle_control_point() {
  if(currShape != curve){
    currPoints = currShape->get_points();
    if(controlPoint < currPoints.size()){
      controlPoint++;
    } else {
      controlPoint = 0;
    }
  } else {
    if(controlPoint < bezierPoints.size() - 1){
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
    currCenter = currShape->get_center();
    currRadiusX = currShape->get_scaleX();
    currSegments = currShape->get_segments();
    currLOD = currShape->get_lod();

    if(currShape == ellipse){
      currShape->set_segments(triCount); // For the ellipse (ie fan) segments and triangles are essentially the same
      if(currLOD < ((float)currSegments*0.01f)){
        currLOD = ((float)currSegments*0.01f);
      }
    } else if(currShape == quad){
      currShape->set_segments(triCount/2); // 1 segment per 2 triangles for the quad
      currRadiusY = currShape->get_scaleY();
      currThickness = currLOD;
    } else if(currShape == curve){
      if(currSegments > 30){
        currSegments = 5;
      }
    } else {
      if(currLOD < ((float)currSegments*0.01f)){
        currLOD = ((float)currSegments*0.01f);
      }
    }

    currShapeColor = currShape->get_shape_fill_color();
    
    currShape->set_points(renderer.get_ellipse_points(currCenter, currRadiusX, currRadiusY, currSegments));
    currPoints = currShape->get_points();

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

    if(currShape == ellipse){
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



    if(currShape == ellipse){

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
        "L: Switch Example\n",
        /*"RAM %dKB/%dKB",*/
        currRadiusX*2, // For a ellipse, both scale values are the same and used to change the diameter of the polygon
        rotationDegrees,
        vertCount+1, // All triangles in the fan use the center vertex and previous vertex, so only accumulate 1 per draw, then add center here
        currLOD,
        triCount,
        display_get_fps(),
        drawTime /*,
        (ramUsed / 1024), (get_memory_size() / 1024)*/
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
        "L: Switch Example\n",
        /*"RAM %dKB/%dKB",*/
        currRadiusX*2,
        currRadiusY*2,
        rotationDegrees,
        vertCount, // Always 4 vertices per quad
        triCount, // Always 2 triangles per quad
        display_get_fps(),
        drawTime/*,
        (ramUsed / 1024), (get_memory_size() / 1024)*/
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
        "L: Switch Example\n",
        /*"RAM %dKB/%dKB",*/
        controlPoint+1, // Point being transformed, where the last of the current Points is the center of the fan
        5,
        rotationDegrees,
        //currLOD,
        currSegments,
        //2 * (currSegments + 1),
        fillTris,
        currTris,
        display_get_fps(),
        drawTime/*,
        (ramUsed / 1024), (get_memory_size() / 1024)*/
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
        "L: Switch Example\n",
        /*"RAM %dKB/%dKB",*/
        currRadiusX*2,
        currRadiusY*2,
        rotationDegrees,
        currSegments,
        controlPoint+1, // Point being transformed, where the last of the current Points is the center of the fan
        (currPoints.size()+1), // Always triangles + center
        currSegments, // Always verts - center
        display_get_fps(),
        drawTime/*,
        (ramUsed / 1024), (get_memory_size() / 1024)*/
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

  return 0;
}
