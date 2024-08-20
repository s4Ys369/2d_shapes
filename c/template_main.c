#include <libdragon.h>

#include "examples/globals.h"
#include "examples/control.h"

#include "examples/bezier.h"
#include "examples/circle.h"
#include "examples/quad.h"
#include "examples/fan.h"

#include "examples/chain.h"
#include "examples/snake.h"

#include "rspq_constants.h"
#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
#include "rspq_profile.h"
static rspq_profile_data_t profile_data;
#endif // RSPQ_PROFILE

//DEFINE_RSP_UCODE(rsp_rdpq_fan);
//uint32_t fan_add_id;

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

  //rspq_init();
  //void* ovlState  = UncachedAddr(rspq_overlay_get_state(&rsp_rdpq_fan));
  //memset(ovlState, 0, 0x10); // One vertex is 0x7, so doubled and aligned to 4
  //fan_add_id = rspq_overlay_register(&rsp_rdpq_fan);

#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
  profile_data.frame_count = 0;
  rspq_profile_start();
#endif // RSPQ_PROFILE

  joypad_init();

  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));


  // Initialize acummulators
  accums_init();

  // Calculated available RAM
  mem_info = mallinfo(); // Setting this every frame leads to memory leak
  ldRAM = 916; // Precalculated RAM used by Libdragon
  totalRAM = (get_memory_size() / 1024) - ldRAM; // Either 3180 or 7276

}

// Main function with rendering loop
int main() {
  setup();
  bootTime = get_ticks_ms();

  for (;;) {

    display_set_fps_limit(0); // Disable limiter

    firstTime = get_ticks_ms();// set loop time

    rdpq_attach(display_get(), &disp);
    rdpq_clear(GREY);
    rdpq_clear_z(0xFFFC);

    rdpq_sync_pipe();
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(0);

    dispTime = get_ticks_ms() - firstTime; // set display time
    secondTime = get_ticks_ms();

    joypad_poll();
    joypad_inputs_t input = joypad_get_inputs(JOYPAD_PORT_1);
    joypad_buttons_t keys = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    joypad_buttons_t keysDown = joypad_get_buttons_held(JOYPAD_PORT_1);

    // RAM used from RDPQ Demo
    ramUsed = mem_info.uordblks - (size_t) (((display_get_width() * display_get_height()) * 2) - ((unsigned int) HEAP_START_ADDR - 0x80000000) - 0x10000);
    ramUsed = (ramUsed / 1024) - ldRAM;

    stickX = (float)input.stick_x;
    stickY = (float)input.stick_y;

    jpTime = get_ticks_ms() - secondTime; // set input time

//=========== ~ UPDATE ~ ==============//

//=========== ~ CONTROLS ~ ==============//

//=========== ~ UI ~ =============//

    uint32_t frameLimit = 59;

    if(frameCounter > frameLimit){
      drawTime = ((get_ticks_ms() - secondTime) + dispTime + jpTime); // CPU time after draw and transform
      frameCounter = 0;
    }
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 20, 20,
      "Verts: %u\n"
      "Tris: %u\n"
      "FPS: %.2f\n"
      "CPU Time: %lldms\n"
      "RAM:\n"
      " Used/Free:\n"
      " %dKB/%dKB\n",
      vertCount,
      triCount,
      display_get_fps(),
      drawTime,
      ramUsed, totalRAM
    );
    
    // Reset acummulators
    accums_reset();

    frameCounter++;
    
    rdpq_detach_show();

#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
    rspq_profile_next_frame();

  // Every second we profile the RSPQ
    if(frameCounter > frameLimit){
      rspq_profile_dump();
      rspq_profile_reset();    
    }

    rspq_profile_get_data(&profile_data);
#endif // RSPQ_PROFILE

  }

  //=========== ~ CLEAN UP ~ =============//
  return 0;
}
