#include <libdragon.h>
#include "utils.h"

// Utility function definitions

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

void add_vertex(float** vertices, int* vertex_count, float x, float y) {
    // Increase the size of the vertices array
    *vertices = (float*)realloc(*vertices, sizeof(float) * (*vertex_count + 2));
    if (*vertices == NULL) {
        debugf("Vertex allocation failed\n");
        return;
    }

    // Add the new vertex
    (*vertices)[*vertex_count] = x;
    (*vertices)[*vertex_count + 1] = y;
    *vertex_count += 2;
}

void add_index(int** indices, int* index_count, int index) {
    // Increase the size of the indices array
    *indices = (int*)realloc(*indices, sizeof(int) * (*index_count + 1));
    if (*indices == NULL) {
        debugf("Index allocation failed\n");
        return;
    }

    // Add the new index
    (*indices)[*index_count] = index;
    *index_count += 1;
}

// Function to create indices for a triangle fan
int* create_triangle_fan_indices(int segments, int* index_count) {
    if (segments < 1) {
        segments = 1;
    }

    // Initialize index array
    int* indices = NULL;
    *index_count = segments * 3; // Each triangle uses 3 indices

    // Add indices for the triangle fan
    for (int i = 1; i <= segments; ++i) {
        add_index(&indices, index_count, 0); // Center vertex
        add_index(&indices, index_count, i); // Current perimeter vertex
        add_index(&indices, index_count, (i % segments) + 1); // Next perimeter vertex
    }

    return indices;
}
