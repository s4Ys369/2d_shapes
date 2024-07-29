#include <libdragon.h>
#include "Point.h"
#include "Utils.h"

// Global variables
surface_t disp;
int example;
float stickX;
float stickY;

// Initialize libdragon
void setup() {

  debug_init_isviewer();
  debug_init_usblog();
    
  dfs_init(DFS_DEFAULT_LOCATION);

  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
  disp = surface_alloc(FMT_RGBA16, display_get_width(), display_get_height());

  rdpq_init();
  //rdpq_debug_start();

  joypad_init();

  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));

  example = 0;
  triCount = 0;
  stickX = 0.0f;
  stickY = 0.0f;

}

// Main rendering function
void draw() {

  switch (example) {
    case 0:
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

    if(keysDown.b){

    }

    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 200, "Tris: %u", triCount);
    triCount = 0;
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 220, "FPS: %.2f", display_get_fps());

    rdpq_detach_show();
  }

  return 0;
}
