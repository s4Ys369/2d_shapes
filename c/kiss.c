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
Shape* fan;
size_t controlPoint = 0;


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
  if (!currShape) {
    debugf("Failed to allocate currShape\n");
    return;
  }
  shape_init(currShape);

  init_point_array(currShape->currPoints);
  if (!currShape->currPoints) {
    debugf("Failed to initialize currShape->currPoints\n");
    return;
  }

  currPoints = (PointArray*)malloc_uncached(sizeof(PointArray));
  if (!currPoints) {
    debugf("Failed to allocate currPoints\n");
    return;
  }
  init_point_array_from_points(currPoints, currShape->currPoints->points, currShape->currPoints->count);

  currShapeColor = get_fill_color(currShape);
  currCenter = get_center(currShape);

  previousPoints = (PointArray*)malloc_uncached(sizeof(PointArray));
  if (!previousPoints) {
    debugf("Failed to allocate previousPoints\n");
    return;
  }
  init_point_array(previousPoints);

  // Circle
  fan = (Shape*)malloc_uncached(sizeof(Shape));
  if (!fan) {
    debugf("Failed to allocate circle\n");
    return;
  }
  fan2_init(fan, screenCenter, 20.0f, 20.0f, 5, BLUE);

}

// Main rendering function
void draw() {
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

void reset_example() {
  currAngle = 0;
  set_center(currShape, screenCenter);
  set_scaleX(currShape, 20.0f);
  set_scaleY(currShape, 20.0f);
  set_lod(currShape, 0.05f);
  set_segments(currShape, 5);
  controlPoint = 0;
}

void switch_example() {
  if (++example > 1) {
    example = 0;
    reset_example();
  }
}

void increase_scale(Shape *currShape) {
  if(get_scaleX(currShape) < screenWidth && get_scaleY(currShape) < screenHeight){
    set_scaleX(currShape, get_scaleX(currShape) + 1.0f);
    set_scaleY(currShape, get_scaleY(currShape) + 1.0f);
  } else {
    set_scaleX(currShape, 1.0f);
    set_scaleY(currShape, 1.0f);
  }
}

void decrease_scale(Shape *currShape) {
  if(get_scaleX(currShape) > 1.0f && get_scaleY(currShape) > 1.0f){
    set_scaleX(currShape, get_scaleX(currShape) - 1.0f);
    set_scaleY(currShape, get_scaleY(currShape) - 1.0f);
  } else {
    set_scaleX(currShape, screenWidth);
    set_scaleY(currShape, screenHeight);
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
    set_scaleX(currShape, currentScaleY - 0.1f);
  } else {
    set_scaleX(currShape, screenHeight);
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
  //if(currShape != (Shape*)curve){
    if(controlPoint < currPoints->count){
      controlPoint++;
    } else {
      controlPoint = 0;
    }
  //} else {
  //  if(controlPoint < bezierPoints->count - 1){
  //    controlPoint++;
  //  } else {
  //    controlPoint = 0;
  //  }
  //}
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
    ramUsed = mem_info.uordblks - (size_t) (((screenWidth * screenHeight) * 2) - ((unsigned int) HEAP_START_ADDR - 0x80000000) - 0x10000);
    if (ramUsed < 0) {
      debugf("Calculated RAM usage is negative: %d\n", ramUsed);
      ramUsed = 0; // Set to zero or handle the error as needed
    }

    stickX = (float)input.stick_x;
    stickY = (float)input.stick_y;

    jpTime = get_ticks_ms() - secondTime; // set input time

    //if (keys.l) { // CHANGE: L for console
    //  switch_example(); 
    //}

    if (keys.start) {
      reset_example();
    }


//=========== ~ UPDATE ~ ==============//

    draw();

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

    // Adjust single scale shape
    if(keysDown.r){
      increase_scale(currShape);
    }
    if(keysDown.z){ // CHANGE: Z for console
      decrease_scale(currShape);
    }


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


//=========== ~ UI ~ =============//

    if(frameCounter > 59){
      drawTime = ((get_ticks_ms() - secondTime) + dispTime + jpTime); // CPU time after draw and transform
      frameCounter = 0;
    }


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
  free_point_array(previousPoints);
  free_point_array(currPoints);
  return 0;
}
