#include <libdragon.h>
#include "Point.h"
#include "Render.h"
#include "Shape.h"
#include "Utils.h"

// Global variables
surface_t disp;
int example;
int triCount;
int vertCount;
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
float currLOD = 0.0f;
float currAngle = 0.0f;
std::vector<Point> currPoints; 
color_t currShapeColor = BLACK;

int ramUsed = 0;

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

  ellipse = new Shape(Point(screenWidth/2,screenHeight/2), 20.0f, 1.0f, RED);
  line = new Shape(Point(screenWidth/2,screenHeight/2), 10.0f, 1.0f, GREEN);
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
      currLOD = currShape->get_lod();
      currShapeColor = currShape->get_shape_fill_color();
      renderer.set_fill_color(currShapeColor);
      currShape->set_points(renderer.get_ellipse_points(currCenter, currRadiusX, currRadiusY, currSegments));
      renderer.draw_ellipse(currCenter.x, currCenter.y, currRadiusX, currRadiusY, currAngle, currLOD);
      currPoints = currShape->get_points();
      break;
    case 1:
      currShape = line;
      line->resolve(stickX, stickY);
      currCenter = currShape->get_center();
      currRadiusX = currShape->get_scaleX();
      currRadiusY = currShape->get_scaleY();
      currThickness = currShape->get_lod();
      currShapeColor = currShape->get_shape_fill_color();
      currShape->set_segments(1); // Initialized as 3 for ellipse, but a line has only one segment per draw
      renderer.set_fill_color(currShapeColor);
      renderer.draw_line(
        currCenter.x-currRadiusX, currCenter.y-currRadiusY, 
        currCenter.x+currRadiusX, currCenter.y+currRadiusY, 
        currAngle,
        currThickness
      );
      break;
    case 2:
    //
      break;
  }

}

// Handles switching examples by incrementing an integer, and wrapping back around to the first 
void switch_example() {
  if (++example > 1) {
    example = 0;
  }
}

void increase_scale(Shape *currShape) {
  if(currShape->get_scaleX() < 100.0f && currShape->get_scaleY() < 100.0f){
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
    currShape->set_scaleX(100.0f);
    currShape->set_scaleY(100.0f);
  }
}

void increase_x_scale(Shape *currShape) {
  float currentScaleX = currShape->get_scaleX();
  if(currentScaleX < 500.0f){
    currShape->set_scaleX(currentScaleX + 1.0f);
  } else {
    currShape->set_scaleX(0.0f);
  }
}

void decrease_x_scale(Shape *currShape) {
  float currentScaleX = currShape->get_scaleX();
  if(currentScaleX > 0.0f){
    currShape->set_scaleX(currentScaleX - 1.0f);
  } else {
    currShape->set_scaleX(500.0f);
  }
}

void increase_y_scale(Shape *currShape) {
  float currentScaleY = currShape->get_scaleY();
  if(currentScaleY < 500.0f){
    currShape->set_scaleY(currentScaleY + 1.0f);
  } else {
    currShape->set_scaleY(0.0f);
  }
}

void decrease_y_scale(Shape *currShape) {
  float currentScaleY = currShape->get_scaleY();
  if(currentScaleY > 0.0f){
    currShape->set_scaleY(currentScaleY - 1.0f);
  } else {
    currShape->set_scaleY(500.0f);
  }
}

void increase_lod(Shape *currShape) {
  if(currShape->get_lod() < 1.0f){
    currShape->set_lod(currShape->get_lod() + 0.01f);
  } else {
    currShape->set_lod(0.01f);
  }
}

void decrease_lod(Shape *currShape) {
  if(currShape->get_lod() >= 0.02f){
    currShape->set_lod(currShape->get_lod() - 0.01f);
  } else {
    currShape->set_lod(1.0f);
  }
}

void increase_thickness(Shape *currShape) {
  if(currShape->get_lod() < 100.0f){
    currShape->set_lod(currShape->get_lod() + 0.1f);
  } else {
    currShape->set_lod(0.1f);
  }
}

void decrease_thickness(Shape *currShape) {
  if(currShape->get_lod() >= 0.2f){
    currShape->set_lod(currShape->get_lod() - 0.1f);
  } else {
    currShape->set_lod(100.0f);
  }
}

void scale_segments(Shape *currShape) {
  if(currShape != ellipse){
  if(currShape->get_segments() < 100){
    currShape->set_segments(currShape->get_segments() + 1);
  } else {
    currShape->set_segments(3);
  }
  } else {
    increase_lod(currShape);
  }
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

    // Fazana's Puppyprint RAM used
    struct mallinfo mem_info = mallinfo();
    ramUsed = mem_info.uordblks - (size_t) (((display_get_width() * display_get_height()) * 2) - ((unsigned int) HEAP_START_ADDR - 0x80000000) - 0x10000);

    stickX = (float)input.stick_x;
    stickY = (float)input.stick_y;

    if (keys.z) {
      switch_example();
    }

    draw();

    // Shape update
    currCenter = currShape->get_center();
    currRadiusX = currShape->get_scaleX();
    currRadiusY = currShape->get_scaleY();
    if(currShape == ellipse){
      currShape->set_segments(triCount); // For the ellipse (ie fan) segments and triangles are essentially the same
    } else {
      currShape->set_segments(triCount/2); // 1 segment per 2 triangles for the line
    }
    currSegments = currShape->get_segments();
    currLOD = currShape->get_lod();
    currShapeColor = currShape->get_shape_fill_color();
    if(currShape == line){
      currThickness = currLOD;
    }
    currShape->set_points(renderer.get_ellipse_points(currCenter, currRadiusX, currRadiusY, currSegments));
    currPoints = currShape->get_points();

    // Add rotation
    float rotation = (float)M_PI/18; // ~10 degrees
    if(keysDown.a){
      currAngle += rotation;
    } else if (keysDown.b) {
      currAngle -= rotation;
    }

    // Adjust single scale shape
    if(keysDown.l){
      increase_scale(currShape);
    }
    if(keysDown.r){
      decrease_scale(currShape);
    }

    // Fine tunes individual scales
    if(keysDown.c_up){
      increase_y_scale(currShape);
    }
    if(keysDown.c_down){
      decrease_y_scale(currShape);
    }
    if(keysDown.c_left){
      increase_x_scale(currShape);
    }
    if(keysDown.c_right){
      decrease_x_scale(currShape);
    }

    // Adjusts LOD
    if(currShape == ellipse){
      if(keys.d_up){
        increase_lod(currShape);
      }
      if(keys.d_down){
        decrease_lod(currShape);
      }

      //=========== ~ UI ~ =============//

      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 120, 
        "Diameter: %.2f\n"
        "Rotation: %.2f\n"
        "Verts: %u\n"
        "LOD: %.2f\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "RAM %dKB/%dKB",
        currRadiusX, // For a ellipse, both scale values are the same and used to change the diameter of the polygon
        currAngle,
        vertCount+1, // All triangles in the fan use the center vertex and previous vertex, so only accumulate 1 per draw, then add center here
        currLOD,
        triCount,
        display_get_fps(),
        (ramUsed / 1024), (get_memory_size() / 1024)
      );
    } else {
      if(keysDown.d_up){
        increase_thickness(currShape);
      }
      if(keysDown.d_down){
        decrease_thickness(currShape);
      }
      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 120, 
        "Length: %.2f\n"
        "Rotation: %.2f\n"
        "Segments: %u\n"
        "Verts: %u\n"
        "Thickness: %.2f\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "RAM %dKB/%dKB",
        currRadiusX, // For a line, both scale values are the same and used to change the length of the quad
        currAngle,
        currSegments, // 1 segment per 2 triangles
        vertCount, // Always 4 vertices per line
        currLOD,
        triCount, // Always 2 triangles per line
        display_get_fps(),
        (ramUsed / 1024), (get_memory_size() / 1024)
      );
    }

    triCount = 0;
    vertCount = 0;

    rdpq_detach_show();
  }

  return 0;
}
