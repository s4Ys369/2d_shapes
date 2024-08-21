#include <libdragon.h>
#include <malloc.h>

#include "rspq_constants.h"
#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
#include "rspq_profile.h"
static rspq_profile_data_t profile_data;
#endif // RSPQ_PROFILE

// Global variables
surface_t disp;
float stickX, stickY;
uint32_t screenWidth, screenHeight, frameCounter;
struct mallinfo mem_info;
int ramUsed = 0;
int totalMem = 0;

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
  frameCounter = 0;
  stickX = 0.0f;
  stickY = 0.0f;

  mem_info = mallinfo();
  totalMem = (get_memory_size() / 1024);

}

// Main function with rendering loop
int main() {
  setup();

  for (;;) {

    if(frameCounter > 59)frameCounter = 0;

    rdpq_attach(display_get(), &disp);
    rdpq_clear(RGBA32(128, 128, 128, 255));

    rdpq_set_mode_standard();

    joypad_poll();
    joypad_inputs_t input = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t keys = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    joypad_buttons_t keysDown = joypad_get_buttons_held(JOYPAD_PORT_1);

    // RAM used from RDPQ Demo
    ramUsed = mem_info.uordblks - (size_t) (((screenWidth * screenHeight) * 2) - ((unsigned int) HEAP_START_ADDR - 0x80000000) - 0x10000);
    if (ramUsed < 0) {
      debugf("Calculated RAM usage is negative: %d\n", ramUsed);
      ramUsed = 0; // Set to zero or handle the error as needed
    }

    stickX = (float)input.stick_x;
    stickY = (float)input.stick_y;

//=========== ~ UPDATE ~ ==============//

//=========== ~ CONTROLS ~ ==============//

//=========== ~ UI ~ =============//

    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 20, "RAM %dKB/%dKB", (ramUsed / 1024), totalMem);

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
  return 0;
}
