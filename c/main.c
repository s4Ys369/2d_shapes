#include <libdragon.h>

#include "examples/globals.h"
#include "examples/control.h"
#include "examples/bezier.h"
#include "examples/circle.h"
#include "examples/quad.h"
#include "examples/fan.h"

#include "rspq_constants.h"
#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
#include "rspq_profile.h"
static rspq_profile_data_t profile_data;
#endif // RSPQ_PROFILE

// Texture test
static sprite_t *test_sprite;

DEFINE_RSP_UCODE(rsp_rdpq_fan);
uint32_t fan_add_id;

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

  rspq_init();
  void* ovlState __attribute__((unused)) = UncachedAddr(rspq_overlay_get_state(&rsp_rdpq_fan));
  //memset(ovlState, 0, 0xA7C); // Size of common code hasn't been matched: text 2068 | 0x814, data 616 | 0x268, total 2684 | 0xA7C
  fan_add_id = rspq_overlay_register(&rsp_rdpq_fan);

#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
  profile_data.frame_count = 0;
  rspq_profile_start();
#endif // RSPQ_PROFILE

  joypad_init();

  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));

  // Texture test
  test_sprite = sprite_load("rom:/n64brew.sprite");
  rdpq_sprite_upload(TILE0, test_sprite, NULL);

  accums_init();
  shape_control_init();
  create_circle();
  create_quad();
  create_fan();
  create_bezier();

  // For quick fan testing
  example = 2;

  
  
}

// Main rendering function
void draw() {
  
  switch (example) {
    case 0:
      circle_draw();
      break;
    case 1:
      quad_draw();
      break;
    case 2:
      fan_draw();
      break;
    case 3:
      bezier_draw();
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
    set_segments(currShape, 3);
    currPoints = get_points(fan);
    controlPoint = currPoints->count;
  } else if (currShape == curve) {
    set_center(currShape, screenCenter);
    set_scaleX(currShape, 20.0f);
    set_scaleY(currShape, 20.0f);
    set_thickness(currShape, 2.0f);
    set_segments(currShape, 10);
    controlPoint = 0;
    resetCurve = 1;
  } else { // Quad
    set_center(currShape, screenCenter);
    set_scaleX(currShape, 20.0f);
    set_scaleY(currShape, 20.0f);
    set_thickness(currShape, 0.01f);
  }
}

void switch_example() {
  reset_example();
  if (++example > 3) {
    example = 0;
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
        cycle_bezier_points();
      }
      if(keys.c_left){
        cycle_bezier_points();
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
        "Quad\n"
        "Width: %.0fpx\n"
        "Height: %.0fpx\n"
        "Rotation: %.0f\n"
        "Verts: %u\n"
        "Tris: %u\n"
        "FPS: %.2f\n"
        "CPU Time: %lldms\n"
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

      rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 14, // sorry overscan sufferers
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
        vertCount - 14, // Subtract the UX circle's verts
        triCount - 13, // Subtract the UX circle's tris
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
      rspq_profile_dump();
      rspq_profile_reset();    
    }

    rspq_profile_get_data(&profile_data);
#endif // RSPQ_PROFILE

  }

  //=========== ~ CLEAN UP ~ =============//
  free_uncached(currShape);
  free_uncached(circle);
  free_uncached(quad);
  free_uncached(fan);
  free_uncached(curve);
  free_uncached(curve2);
  free_uncached(bezierPoints);
  free_uncached(basePoints);
  free_uncached(currPoints);
  return 0;
}
