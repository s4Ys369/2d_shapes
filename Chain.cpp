#include <libdragon.h>
#include "Chain.h"
#include "Utils.h"

Chain::Chain(PVector origin, int jointCount, int linkSize)
    : Chain(origin, jointCount, linkSize, TWO_PI) {}

Chain::Chain(PVector origin, int jointCount, int linkSize, float angleConstraint)
    : linkSize(linkSize), angleConstraint(angleConstraint) {
    joints.reserve(jointCount); // Reserve space for efficiency
    angles.reserve(jointCount - 1); // One less angle than joints
    joints.push_back(origin.copy());
    angles.push_back(0);
    for (int i = 1; i < jointCount; i++) {
        joints.push_back(PVector::sub(joints[i - 1], PVector(0, this->linkSize)));
        angles.push_back(0);
    }
}

void Chain::resolve(PVector pos) {
    angles[0] = PVector::sub(pos, joints[0]).heading();
    joints[0] = pos;
    for (size_t i = 1; i < joints.size(); i++) {
        float curAngle = PVector::sub(joints[i - 1], joints[i]).heading();
        angles[i] = constrainAngle(curAngle, angles[i - 1], angleConstraint);
        joints[i] = PVector::sub(joints[i - 1], PVector::fromAngle(angles[i]).setMag(linkSize));
    }
}

void Chain::fabrikResolve(PVector pos, PVector anchor) {
    // Forward pass
    joints[0] = pos;
    for (size_t i = 1; i < joints.size(); i++) {
        joints[i] = constrainDistance(joints[i], joints[i - 1], linkSize);
    }

    // Backward pass
    joints[joints.size() - 1] = anchor;
    for (size_t i = joints.size() - 2; i >= 0; i--) {
        joints[i] = constrainDistance(joints[i], joints[i + 1], linkSize);
    }
}

void Chain::draw_line(float x1, float y1, float x2, float y2) {
    // Convert the line into a thin rectangle (a series of triangles)
    const float thickness = 4.0f; // Adjust this value as needed for strokeWeight
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);

    // Normalize the direction vector
    dx /= length;
    dy /= length;

    // Perpendicular vector for thickness
    float px = -dy * thickness / 4;
    float py = dx * thickness / 4;

    float v1[] = { x1 + px, y1 + py };
    float v2[] = { x1 - px, y1 - py };
    float v3[] = { x2 + px, y2 + py };
    float v4[] = { x2 - px, y2 - py };

    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    rdpq_triangle(&TRIFMT_FILL, v2, v3, v4);
}

void Chain::draw_ellipse(float cx, float cy, float rx, float ry) {
  const int segments = 10;
  float theta = 2 * M_PI / float(segments);
  float cos_theta = cosf(theta);
  float sin_theta = sinf(theta);

  float x = rx;
  float y = 0;

  for (int i = 0; i < segments; ++i) {
    float next_x = cos_theta * x - sin_theta * y;
    float next_y = sin_theta * x + cos_theta * y;

    float v1[] = { cx, cy };
    float v2[] = { cx + x, cy + y };
    float v3[] = { cx + next_x, cy + next_y };

    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);

    x = next_x;
    y = next_y;
  }
}

void Chain::display() {

    // Draw lines between joints
    for (size_t i = 0; i < joints.size() - 1; ++i) {
        PVector startJoint = joints[i];
        PVector endJoint = joints[i + 1];
        rdpq_set_prim_color(BLACK);
        draw_line(startJoint.x, startJoint.y, endJoint.x, endJoint.y);
        rdpq_set_prim_color(BLACK);
        draw_ellipse(joints[i].x, joints[i].y, 4.0f, 1.0f);
        rdpq_set_prim_color(YELLOW);
        draw_ellipse(joints[i].x, joints[i].y, 3.0f, 1.0f);
    }
}