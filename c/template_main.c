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
Point screenCenter;
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

//=========== ~ UPDATE ~ ==============//



//=========== ~ CONTROLS ~ ==============//

//=========== ~ UI ~ =============//

    if(frameCounter > 59){
      drawTime = ((get_ticks_ms() - secondTime) + dispTime + jpTime); // CPU time after draw and transform
      frameCounter = 0;
    }
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 20, "RAM %dKB/%dKB", (ramUsed / 1024), (get_memory_size() / 1024));
    
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
  return 0;
}