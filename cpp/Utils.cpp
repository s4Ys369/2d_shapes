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
* - Port from PDE to C++
* - Focusing on optimization for N64
*
*
* Original source: https://github.com/argonautcode/animal-proc-anim/blob/main/Util.pde
*/


#include <libdragon.h>
#include "Utils.h"

// Start of anim-proc-anim functions //

// Function to adjust a point's position to be within a certain range relative to an anchor point, constrained by a given amount
Point constrain_distance(Point pos, Point anchor, float constraint) {
    return Point::sum(anchor, Point::sub(pos, anchor).set_mag(constraint));
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
float apply_deadzone(float value) {

  float sign = copysignf(1.0f, value);

  float absValue = fabsf(value);

  float remappedValue = fmaxf(absValue - DEADZONE, 0.0f) / (1.0f - DEADZONE);

  return sign * remappedValue * (absValue >= DEADZONE);
}
