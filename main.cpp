#include <libdragon.h>
#include "Point.h"
#include "Render.h"
#include "Shape.h"
#include "Utils.h"

// Global variables
surface_t disp;
int example;
int triCount;
float stickX;
float stickY;

// Shape pointers
Shape* currShape;
Shape* ellipse;
Shape* line;
Shape* fan;

// Local variables
Point currCenter = 0;
float currRadiusX = 0;
float currRadiusY = 0;
float currThickness = 0;
int currSegments = 0;
std::vector<Point> currPoints; 
color_t currShapeColor = BLACK;

// Initialize libdragon
void setup() {

  debug_init_isviewer();
  debug_init_usblog();
    
  dfs_init(DFS_DEFAULT_LOCATION);

  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
  float screenWidth = display_get_width();
  float screenHeight = display_get_height();
  disp = surface_alloc(FMT_RGBA16, screenWidth, screenHeight);

  rdpq_init();
  //rdpq_debug_start();

  joypad_init();

  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));

  example = 0;
  triCount = 0;
  stickX = 0.0f;
  stickY = 0.0f;

  ellipse = new Shape(Point(screenWidth/2,screenHeight/2), 1.0f, 1.0f, 14, RED);
  line = new Shape(Point(screenWidth/2,screenHeight/2), 1.0f, 1, GREEN);
  fan = new Shape(Point(screenWidth/2,screenHeight/2), 1.0f, 10, BLUE);

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
      currShapeColor = currShape->get_shape_fill_color();
      renderer.set_fill_color(currShapeColor);
      renderer.draw_ellipse(currCenter.x, currCenter.y, currRadiusX, currRadiusY, currSegments);
      break;
    case 1:
      break;
    case 2:
      break;
  }

}

// Handles switching examples by incrementing an integer, and wrapping back around to the first 
void switch_example() {
  if (++example > 2) {
    example = 0;
  }
}

void scale_shape(Shape *currShape) {
  if(currShape->get_scaleX() < (display_get_height()/2) && currShape->get_scaleY() < (display_get_width()/2)){
    currShape->set_scaleX(currShape->get_scaleX() + 1.0f);
    currShape->set_scaleY(currShape->get_scaleY() + 1.0f);
  } else {
    currShape->set_scaleX(1.0f);
    currShape->set_scaleY(1.0f);
  }
}

void scale_segments(Shape *currShape) {
  if(currShape->get_segments() < 100){
    currShape->set_segments(currShape->get_segments() + 1);
  } else {
    currShape->set_segments(3);
  }
}

void rotate_points(std::vector<Point>& currPoints, float angle) {
  std::vector<Point> rotatedPoints;
  Point center = currShape->get_center();
    
  for (const auto& point : currPoints) {
    // Translate point back to origin
    Point translatedPoint =  Point::sub(point, center);
        
    // Rotate the translated point
    Point rotatedPoint = Point::rotate(translatedPoint, angle);
        
    // Translate point back to its original position
    rotatedPoint = Point::add(rotatedPoint, center);
        
    rotatedPoints.push_back(rotatedPoint);
  }
    
  currPoints = rotatedPoints;
}

// Main function with rendering loop
int main() {
  setup();

  while (1) {

    rdpq_attach(display_get(), &disp);
    rdpq_clear(WHITE);
    rdpq_clear_z(0xFFFC);

    rdpq_sync_pipe();
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);

    joypad_poll();
    joypad_inputs_t input = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t keys = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    joypad_buttons_t keysDown = joypad_get_buttons_held(JOYPAD_PORT_1);

    stickX = (float)input.stick_x;
    stickY = (float)input.stick_y;

    if (keys.z) {
      switch_example();
    }

    draw();
    currCenter = currShape->get_center();
    currRadiusX = currShape->get_scaleX();
    currRadiusY = currShape->get_scaleY();
    currSegments = currShape->get_segments();
    currShapeColor = currShape->get_shape_fill_color();
    renderer.get_ellipse_points(currCenter.x, currCenter.y, currRadiusX, currRadiusY, currSegments, currPoints);

    if(keysDown.a){
      rotate_points(currPoints, M_PI/12);
    }

    if(keysDown.l){
      scale_shape(currShape);
    }
    if(keysDown.r){
      scale_segments(currShape);
    }

    
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 180, "Number of current points: %u\n", currPoints.size());
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 200, "Tris: %u", triCount);
    triCount = 0;
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 220, "FPS: %.2f", display_get_fps());

    rdpq_detach_show();
  }

  return 0;
}
