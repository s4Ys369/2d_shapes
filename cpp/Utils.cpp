#include <libdragon.h>
#include "Utils.h"

// Utility function definitions

// Function to adjust a point's position to be within a certain range relative to an anchor point, constrained by a given amount
Point constrain_distance(Point pos, Point anchor, float constraint) {
    return Point::sum(anchor, Point::sub(pos, anchor).set_mag(constraint));
}

// Function to normalize an angle to be within the range [0,2pi]
float simplify_angle(float angle) {
    while (angle >= TWO_PI) {
        angle -= TWO_PI;
    }
    while (angle < 0) {
        angle += TWO_PI;
    }
    return angle;
}

// Function to compute the relative angular difference between a given angle and an anchor angle, adjusted by pi
float rel_angle_diff(float angle, float anchor) {
    angle = simplify_angle(angle + M_PI - anchor);
    anchor = M_PI;
    return anchor - angle;
}

// Function to adjust an angle to be within a certain angular range relative to an anchor angle, constrained by a given amount
float constrain_angle(float angle, float anchor, float constraint) {
    if (fabsf(rel_angle_diff(angle, anchor)) <= constraint) {
        return simplify_angle(angle);
    }

    if (rel_angle_diff(angle, anchor) > constraint) {
        return simplify_angle(anchor - constraint);
    }

    return simplify_angle(anchor + constraint);
}

// Function to apply deadzone to a joystick axis input
float apply_deadzone(float value) {
  if (fabsf(value) < DEADZONE) {
    return 0.0f; // Within deadzone, treat as zero
  } else {
    // Remap the value outside the deadzone
    if (value > 0) {
        return (value - DEADZONE) / (1.0f - DEADZONE);
    } else {
      return (value + DEADZONE) / (1.0f - DEADZONE);
    }
  }
}
