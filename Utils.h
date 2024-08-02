#ifndef UTILS_H
#define UTILS_H

#include <libdragon.h>
#include <malloc.h>
#include "Point.h"

// Constants
const float TWO_PI = 2 * M_PI;
const float radiansToDegrees = 180.0f / M_PI;
const float DEADZONE = 20.0f; // Define the deadzone threshold

extern int triCount;
extern int vertCount;
extern int currTris;
extern int fillTris;
extern int currVerts;

// Colors
const color_t RED = (color_t){209, 0, 0, 255};
const color_t ORANGE = (color_t){255, 102, 34, 255};
const color_t YELLOW = (color_t){255, 218, 33, 255};
const color_t GREEN = (color_t){51, 221, 0, 255};
const color_t BLUE = (color_t){17, 51, 204, 255};
const color_t INDIGO = (color_t){34, 0, 102, 255};
const color_t VIOLET = (color_t){51, 0, 68, 255};
const color_t BLACK = (color_t){0,0,0,255};
const color_t WHITE = (color_t){255, 255, 255, 255};

const color_t LIGHT_GREY = (color_t){192, 192, 192, 255};
const color_t GREY = (color_t){128, 128, 128, 255};
const color_t DARK_GREY = (color_t){64, 64, 64, 255};

const color_t TRANSPARENT = (color_t){0, 0, 0, 127};
const color_t T_RED = (color_t){209, 0, 0, 200};
const color_t T_ORANGE = (color_t){255, 102, 34, 200};
const color_t T_YELLOW = (color_t){255, 218, 33, 200};
const color_t T_GREEN = (color_t){51, 221, 0, 200};
const color_t T_BLUE = (color_t){17, 51, 204, 200};
const color_t T_INDIGO = (color_t){34, 0, 102, 200};
const color_t T_VIOLET = (color_t){51, 0, 68, 200};
const color_t T_BLACK = (color_t){0,0,0,200};
const color_t T_WHITE = (color_t){255, 255, 255, 200};
const color_t T_GREY = (color_t){192, 192, 192, 200};

const color_t DARK_RED = (color_t){130, 0, 0, 255};
const color_t DARK_GREEN = (color_t){0, 100, 0, 255};

// Constrain functions
Point constrain_distance(Point pos, Point anchor, float constraint);
float simplify_angle(float angle);
float rel_angle_diff(float angle, float anchor);
float constrain_angle(float angle, float anchor, float constraint);
float apply_deadzone(float value);

#endif // UTILS_H