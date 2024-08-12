/*
* This file includes code from the animal-proc-anim project.
* animal-proc-anim is licensed under the MIT License.
* See the LICENSES directory for the full text of the MIT License.
*
* Original code by argonaut 
* Adapted by s4ys
* August 2024
*
* Description of changes or adaptations made:
* - Port from PDE to C
* - Focusing on optimization for N64
*
*
* Original source: https://github.com/argonautcode/animal-proc-anim/blob/main/Util.pde
*/

#include <libdragon.h>
#include "utils.h"

const float TWO_PI = 2 * M_PI;
const float radiansToDegrees = 180.0f / M_PI;

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

// Start of anim-proc-anim functions //

// Function to adjust a point's position to be within a certain range relative to an anchor point, constrained by a given amount
Point constrain_distance(Point pos, Point anchor, float constraint) {
  // Calculate the vector from anchor to pos
  Point direction = point_sub(&pos, &anchor);
    
  // Set the magnitude of this vector to the constraint distance
  Point constrainedDirection = point_set_mag(&direction, constraint);
    
  // Sum the anchor point and the constrained direction vector
  return point_sum(&anchor, &constrainedDirection);
}

// Function to normalize an angle to be within the range [0,2pi]
float simplify_angle(float angle) {
  // Modulus operation to bring the angle within [0, 2*pi)
  angle = fmodf(angle, TWO_PI);
    
  // If the angle is negative, bring it to the positive equivalent within [0, 2*pi)
  angle += (angle < 0) ? TWO_PI : 0;
    
  return angle;
}

// Function to compute the relative angular difference between a given angle and an anchor angle, adjusted by pi
float rel_angle_diff(float angle, float anchor) {
  float diff = simplify_angle(angle - anchor);
  return fmodf(diff + M_PI, TWO_PI) - M_PI; // Normalize within [-π, π]
}

// Function to adjust an angle to be within a certain angular range relative to an anchor angle, constrained by a given amount
float constrain_angle(float angle, float anchor, float constraint) {
  float diff = rel_angle_diff(angle, anchor);

  // Calculate the clamped difference using mathematical operations
  float clampedDiff = fminf(fmaxf(diff, -constraint), constraint);

  // Compute the constrained angle based on the clamped difference
  float constrainedAngle = anchor + clampedDiff;

  // Normalize the constrained angle within [0, 2*pi)
  return simplify_angle(constrainedAngle);
}

// End of anim-proc-anim functions //

// Function to apply deadzone to a joystick axis input
const float DEADZONE = 20.0f;
float apply_deadzone(float value) {

  float sign = copysignf(1.0f, value);

  float absValue = fabsf(value);

  float remappedValue = fmaxf(absValue - DEADZONE, 0.0f) / (1.0f - DEADZONE);

  return sign * remappedValue * (absValue >= DEADZONE);
}

/* C++ replacements , possibly move to render? */

// Function to add a vertex to a vertex array
void add_vertex(float** vertices, int* vertex_count, float x, float y) {
  float* new_vertices = (float*)realloc(*vertices, sizeof(float) * (*vertex_count + 2));
  if (new_vertices == NULL) {
    debugf("Vertex reallocation failed\n");
    return;
  }
  *vertices = new_vertices;
  (*vertices)[*vertex_count] = x;
  (*vertices)[*vertex_count + 1] = y;
  *vertex_count += 2;
}

// Function to add an index to the index array
void add_index(int** indices, int* index_count, int index) {
  int new_size = *index_count + 1;
  int* new_indices = (int*)realloc(*indices, sizeof(int) * new_size);
  if (new_indices == NULL) {
    debugf("Index reallocation failed\n");
    return;
  }
  *indices = new_indices;

  (*indices)[*index_count] = index;
  *index_count = new_size;
}

// Function to create triangle fan indices
int* create_triangle_fan_indices(int* indices, int segments, int* index_count) {
  *index_count = (segments + 1) * 3 - 3; // Number of indices needed
  indices = (int*)malloc_uncached(*index_count * sizeof(int));

  int center_idx = 0;
  for (int i = 1; i < segments; ++i) {
    indices[(i - 1) * 3] = center_idx;
    indices[(i - 1) * 3 + 1] = i;
    indices[(i - 1) * 3 + 2] = i + 1;
  }
  // Last triangle to close the fan
  indices[(segments - 1) * 3] = center_idx;
  indices[(segments - 1) * 3 + 1] = segments;
  indices[(segments - 1) * 3 + 2] = 1;

  return indices;
}

/* Bernstein polynominal for bezier curve
void get_bezier_points(PointArray* curvePoints, int segments){
  for (int i = 0; i <= segments; ++i) {
    float t = i * step;
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    float x = uuu * p0->x + 3 * uu * t * p1->x + 3 * u * tt * p2->x + ttt * p3->x;
    float y = uuu * p0->y + 3 * uu * t * p1->y + 3 * u * tt * p2->y + ttt * p3->y;

    add_point(&curvePoints, x, y);
  }
}*/
