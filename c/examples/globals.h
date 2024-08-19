#ifndef GLOBALS_H
#define GLOBALS_H

#include <libdragon.h>
#include "../point.h"

// Global variables
surface_t disp;
int example, triCount, vertCount, currVerts, currTris, fillTris;
float stickX, stickY;
uint64_t bootTime, firstTime, secondTime, dispTime, jpTime, drawTime;
uint32_t screenWidth, screenHeight, frameCounter;
Point screenCenter;
size_t controlPoint;

typedef enum {
  CIRCLE,
  QUAD,
  FAN,
  BEZIER,
  SNAKES
} EXAMPLES;

void accums_init(){
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
  controlPoint = 0;
}

#endif // GLOBALS_H