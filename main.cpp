#include <libdragon.h>
#include "Point.h"
#include "Render.h"
#include "Shape.h"
#include "Utils.h"

// Global variables
surface_t disp;
int example, triCount, vertCount;
float stickX, stickY;
uint64_t bootTime, firstTime, secondTime, dispTime, jpTime, drawTime;
uint32_t screenWidth, screenHeight, frameCounter;

// Shape pointers
Shape* currShape;
Shape* ellipse;
Shape* quad;
Shape* fan;
Shape* curve;
Point pointA;
Point pointB;
Point pointC;
Point pointD;


// Local variables
Point currCenter = 0;
float currRadiusX = 0;
float currRadiusY = 0;
float currThickness = 0;
int currSegments = 0;
float currLOD = 0.0f;
float currAngle = 0.0f;
std::vector<Point> currPoints; 
std::size_t controlPoint = 0;
color_t currShapeColor = BLACK;

// Texture test
static sprite_t *test_sprite;

int ramUsed = 0;

// Initialize libdragon
void setup() {

  debug_init_isviewer();
  debug_init_usblog();
  bootTime= 0;
  firstTime = 0;
  secondTime = 0;
  jpTime = 0;
  dispTime = 0;
  drawTime = 0;
  frameCounter = 0;
    
  dfs_init(DFS_DEFAULT_LOCATION);

  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
  screenWidth = display_get_width();
  screenHeight = display_get_height();
  disp = surface_alloc(FMT_RGBA16, screenWidth, screenHeight);

  rdpq_init();
  rdpq_debug_start();

  joypad_init();

  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));

  example = 0;
  triCount = 0;
  vertCount = 0;
  stickX = 0.0f;
  stickY = 0.0f;

  ellipse = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 0.05f, RED);
  quad = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 20.0f, 0.01f, 1, DARK_GREEN);
  fan = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 20.0f, 5, BLUE);
  curve = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 20.0f, 2.0f, 10, ORANGE);

  // Texture test
  test_sprite = sprite_load("rom:/n64brew.sprite");
  rdpq_sprite_upload(TILE0, test_sprite, NULL);

  

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
      currShape->set_segments(1); // Initialized as 3 for ellipse, but a quad has only one segment per draw
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
      currPoints = currShape->get_points();

      renderer.move_point(currPoints, controlPoint, stickX, stickY);
      renderer.rotate_point(currPoints, controlPoint, currCenter, currAngle);
      if(controlPoint == currPoints.size()){
        renderer.move_shape_points(currPoints, stickX, stickY);
        renderer.rotate_shape_points(currPoints, currCenter, currAngle);
      }
      
      renderer.set_fill_color(currShapeColor);
      renderer.draw_fan(currPoints);

      if ( controlPoint < currPoints.size()){
        renderer.set_fill_color(BLACK);
        renderer.draw_ellipse(currPoints[controlPoint].x, currPoints[controlPoint].y, 3.0f, 3.0f, 0.0f, 0.05f);
        renderer.set_fill_color(WHITE);
        renderer.draw_ellipse(currPoints[controlPoint].x, currPoints[controlPoint].y, 2.0f, 2.0f, 0.0f, 0.05f);
      } else {
        renderer.set_fill_color(BLACK);
        renderer.draw_ellipse(currCenter.x, currCenter.y, 3.0f, 3.0f, 0.0f, 0.05f);
        renderer.set_fill_color(WHITE);
        renderer.draw_ellipse(currCenter.x, currCenter.y, 2.0f, 2.0f, 0.0f, 0.05f);
      }
      break;
    case 3:
      currShape = curve;
      curve->resolve(stickX, stickY);
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
      currShapeColor = currShape->get_shape_fill_color();
      renderer.set_fill_color(currShapeColor);


      // Control points
      pointA = Point( (currCenter.x - (currRadiusX*2.0f)), (currCenter.y + currRadiusY) );
      pointB = Point( (currCenter.x - (currRadiusX)), (currCenter.y - currRadiusY*2.0f) );
      pointC = Point( (currCenter.x + (currRadiusX)), (currCenter.y - currRadiusY*2.0f) );
      pointD = Point( (currCenter.x + (currRadiusX*2.0f)), (currCenter.y + currRadiusY) );

      renderer.draw_bezier_curve(
        pointA, pointB, pointC, pointD,
        currSegments, // 3 segments
        currAngle,
        currThickness
      );

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
  currShape->set_points(renderer.get_ellipse_points(currCenter, currRadiusX, currRadiusY, currSegments));
  currPoints = currShape->get_points();
  if(controlPoint < currPoints.size()){
    controlPoint++;
  } else {
    controlPoint = 0;
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

    if (keys.z) {
      switch_example();
    }

    if (keys.start) {
      reset_example();
    }

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

    // Add rotation
    float rotation = (float)(M_PI)/18.0f; // ~10 degrees

    if(keysDown.a){
      currAngle += rotation;
    } else if (keysDown.b) {
      currAngle -= rotation;
    }

    // Adjust single scale shape
    if(keysDown.r){
      increase_scale(currShape);
    }
    if(keysDown.l){ // CHANGE: Z for console?
      decrease_scale(currShape);
    }

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

    float rotationDegrees = currAngle * radiansToDegrees;

    if(fabsf(rotationDegrees) > 360.0f) {
      currAngle = 0;
      rotationDegrees = 0;
    }

    // Adjusts LOD
    if(currShape == ellipse){
      if(keys.d_up){
        increase_lod(currShape);
      }
      if(keys.d_down){
        decrease_lod(currShape);
      }

      // Every econd we profile the CPU time to draw 
      if(frameCounter > 59){
        drawTime = ((get_ticks_ms() - secondTime) + dispTime + jpTime); // time after draw and transform
        frameCounter = 0;
      }

      //=========== ~ UI ~ =============//

      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 100, 
        "Circle\n"
        "\n"
        "Diameter: %.0f px\n"
        "Rotation: %.0f\n"
        "Verts: %u\n"
        "LOD: %.2f\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "Draw Time: %lldms\n",
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

      drawTime = ((get_ticks_ms() - secondTime) + dispTime + jpTime); // time after draw and transform

      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 100,
        "Quad\n"
        "\n"
        "Width: %.0fpx\n"
        "Height: %.0fpx\n"
        "Rotation: %.0f\n"
        "Verts: %u\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "Draw Time: %lldms\n",
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
      if(keysDown.d_up){
        increase_segments(currShape);
      }
      if(keysDown.d_down){
        decrease_segments(currShape);
      }
      if(keysDown.d_right){
        increase_thickness(currShape);
      }
      if(keysDown.d_left){
        decrease_thickness(currShape);
      }

      drawTime = ((get_ticks_ms() - secondTime) + dispTime + jpTime); // time after draw and transform


      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 100,
        "Bezier Curve\n"
        "\n"
        "Width: %.0fpx\n"
        "Height: %.0fpx\n"
        "Rotation: %.0f\n"
        "Thickness %.2f\n"
        "Segments: %u\n"
        "Verts: %u\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "Draw Time: %lldms\n",
        /*"RAM %dKB/%dKB",*/
        currRadiusX*2,
        currRadiusY*2,
        rotationDegrees,
        currLOD,
        currSegments,
        vertCount,
        triCount,
        display_get_fps(),
        drawTime/*,
        (ramUsed / 1024), (get_memory_size() / 1024)*/
      );
    } else {
      if(keysDown.d_up){
        increase_segments(currShape);
      }
      if(keysDown.d_down){
        increase_segments(currShape);
      }
      if(keys.d_right){
        cycle_control_point();
      }

      drawTime = ((get_ticks_ms() - secondTime) + dispTime + jpTime); // time after draw and transform


      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 100,
        "Fan (Transform)\n"
        "\n"
        "Width: %.0fpx\n"
        "Height: %.0fpx\n"
        "Rotation: %.0f\n"
        "Segments: %u\n"
        "Verts: %d/%d\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "Draw Time: %lldms\n",
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

    triCount = 0;
    vertCount = 0;
    firstTime = 0;
    secondTime = 0;
    jpTime = 0;
    dispTime = 0;

    frameCounter++;

    rdpq_detach_show();
  }

  return 0;
}
