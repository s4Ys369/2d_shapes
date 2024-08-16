#ifndef UTILS_H
#define UTILS_H

#include <libdragon.h>
#include <malloc.h>
#include <math.h>
#include "point.h"


// Define whether to use RDPQ Validate
#define DEBUG_RDPQ

// Math constants
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
extern const float TWO_PI;
extern const float radiansToDegrees;

// Accumulators for UI
extern int triCount;
extern int vertCount;
extern int currTris;
extern int fillTris;
extern int currVerts;

// Colors
extern const color_t RED;
extern const color_t ORANGE;
extern const color_t YELLOW;
extern const color_t GREEN;
extern const color_t BLUE;
extern const color_t INDIGO;
extern const color_t VIOLET;
extern const color_t BLACK;
extern const color_t WHITE;

extern const color_t LIGHT_GREY;
extern const color_t GREY;
extern const color_t DARK_GREY;

extern const color_t TRANSPARENT;
extern const color_t T_RED;
extern const color_t T_ORANGE;
extern const color_t T_YELLOW;
extern const color_t T_GREEN;
extern const color_t T_BLUE;
extern const color_t T_INDIGO;
extern const color_t T_VIOLET;
extern const color_t T_BLACK;
extern const color_t T_WHITE;
extern const color_t T_GREY;

extern const color_t DARK_RED;
extern const color_t DARK_GREEN;

// anim-proc-anim functions
Point constrain_distance(Point pos, Point anchor, float constraint);
float simplify_angle(float angle);
float rel_angle_diff(float angle, float anchor);
float constrain_angle(float angle, float anchor, float constraint);

// Controller specific
extern const float DEADZONE;
float apply_deadzone(float value);

// C++ auto constructor replacements
void add_vertex(float** vertices, int* vertex_count, float x, float y);
void add_index(int** indices, int* index_count, int index);
int* create_triangle_fan_indices(int* indices, int segments, int* index_count);

#endif // UTILS_H