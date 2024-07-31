#ifndef UTILS_H
#define UTILS_H

#include <libdragon.h>
#include <malloc.h>
#include "Point.h"

// Constants
const float TWO_PI = 2 * M_PI;
const float radiansToDegrees = 180.0f / M_PI;
const float DEADZONE = 0.02f; // Define the deadzone threshold

extern int triCount;
extern int vertCount;

// Colors
const color_t WHITE = (color_t){255, 255, 255, 255};
const color_t BLACK = (color_t){0, 0, 0, 255};
const color_t RED = (color_t){255, 0, 0, 255};
const color_t DARK_RED = (color_t){128, 0, 0, 255};
const color_t GREEN = (color_t){0, 255, 0, 255};
const color_t DARK_GREEN = (color_t){0, 128, 0, 255};
const color_t BLUE = (color_t){0, 0, 255, 255};
const color_t YELLOW = (color_t){255, 218, 33, 255};

// Constrain functions
Point constrain_distance(Point pos, Point anchor, float constraint);
float simplify_angle(float angle);
float rel_angle_diff(float angle, float anchor);
float constrain_angle(float angle, float anchor, float constraint);
float apply_deadzone(float value);

#endif // UTILS_H