#include <libdragon.h>
#include "Chain.h"
#include "Snake.h"
#include "Utils.h"

int TC = 0;
// Wiggly lil dude
Snake::Snake(PVector origin)
    : spine(origin, 16, 16, M_PI / 8),
      bodyWidth{4.5f, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 1} {}

void Snake::resolve(float mouseX, float mouseY) {

  PVector headPos = spine.joints[0];
  PVector targetPos = headPos;

  // Apply deadzone to the joystick inputs
  float adjustedX = apply_deadzone(mouseX);
  float adjustedY = apply_deadzone(mouseY);

  // Normalize the joystick input to get the direction
  PVector direction(adjustedX, -adjustedY);
  direction.normalize();

  // Determine the target position based on the direction and a fixed magnitude
  float movementMag = 3.5;
  targetPos = PVector::add(headPos, direction.setMag(movementMag));

  spine.resolve(targetPos);
  //debugf("X %.1f\nY %.1f\n", targetPos.x, targetPos.y);
}

float Snake::getBodyWidth(int i) {
  return bodyWidth[i];
}

float Snake::getPosX(int i, float angleOffset, float lengthOffset) {
  return spine.joints[i].x + cosf(spine.angles[i] + angleOffset) * (bodyWidth[i] + lengthOffset);
}

float Snake::getPosY(int i, float angleOffset, float lengthOffset) {
  return spine.joints[i].y + sinf(spine.angles[i] + angleOffset) * (bodyWidth[i] + lengthOffset);
}

void Snake::draw_ellipse(float cx, float cy, float rx, float ry) {
  const int segments = 14;
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
      TC++;

      x = next_x;
      y = next_y;
  }
}

// Function get points around an ellipse
void Snake::get_ellipse_points(float cx, float cy, float rx, float ry, int segments, std::vector<PVector>& points) {
  points.clear(); // Clear the vector to store new points
  for (int i = 0; i <= segments; ++i) {
      float theta = 2.0f * M_PI * float(i) / float(segments);
      float x = rx * cosf(theta);
      float y = ry * sinf(theta);
      points.push_back({cx + x, cy + y});
  }
}


void Snake::draw_snake_shape() {
  size_t vertex_count = 0;
  size_t max_vertices = spine.joints.size() * 2 + 8; // Adjust this as needed
  //debugf("%u", max_vertices);
  PVector* vertices = (PVector*)malloc(max_vertices * sizeof(PVector));
    
  if (!vertices) {
    //debugf("No vertices!");
    return;
  }

  // Right half of the snake
  for (size_t i = 0; i < 14; i++) {
    vertices[vertex_count].x = getPosX(i, M_PI / 2, 0);
    vertices[vertex_count].y = getPosY(i, M_PI / 2, 0);
    vertex_count++;
  }

  // Top of the head (completes the loop)
  vertices[vertex_count].x = getPosX(13, M_PI, 0);
  vertices[vertex_count].y = getPosY(13, M_PI, 0);
  vertex_count++;

  // Left half of the snake
  for (int i = 13; i >= 0; --i) {
    vertices[vertex_count].x = getPosX(i, -M_PI / 2, 0);
    vertices[vertex_count].y = getPosY(i, -M_PI / 2, 0);
    vertex_count++;
  }

  // Add vertices to complete the loop
  vertices[vertex_count].x = getPosX(0, -M_PI / 6, 0);
  vertices[vertex_count].y = getPosY(0, -M_PI / 6, 0);
  vertex_count++;
  vertices[vertex_count].x = getPosX(0, 0, 0);
  vertices[vertex_count].y = getPosY(0, 0, 0);
  vertex_count++;
  vertices[vertex_count].x = getPosX(0, M_PI / 6, 0);
  vertices[vertex_count].y = getPosY(0, M_PI / 6, 0);
  vertex_count++;

  // Calculate the center of the shape
  PVector center = { 0, 0 };
  for (size_t i = 0; i < vertex_count; ++i) {
      center.x += vertices[i].x;
      center.y += vertices[i].y;
  }
  center.x /= vertex_count;
  center.y /= vertex_count;

  // Scale the vertices outward for outline
  float e = 0.03f;
  PVector scaled_vertices[max_vertices];
  float scale = 1.0f + e;
  for (size_t i = 0; i < vertex_count; ++i) {
    scaled_vertices[i] = PVector::scale(center, vertices[i], scale);
  }

  // Draw edges
  for (size_t i = 0; i < vertex_count - 2; ++i) {

    // cast PVector to float[] for `rdpq_triangle`
    float v1[] = { vertices[i].x, vertices[i].y };
    float v2[] = { vertices[i + 1].x, vertices[i + 1].y };
    float v3[] = { vertices[i + 2].x, vertices[i + 2].y };

    float v1S[] = { scaled_vertices[i].x, scaled_vertices[i].y };
    float v2S[] = { scaled_vertices[i + 1].x, scaled_vertices[i + 1].y };
    float v3S[] = { scaled_vertices[i + 2].x, scaled_vertices[i + 2].y };

    // Set outline color and draw
    rdpq_set_prim_color(BLACK); 
    rdpq_triangle(&TRIFMT_FILL, v1S, v2S, v3S);
    TC++;

    // Set edge color and draw
    rdpq_set_prim_color(RED);
    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    TC++;
  }

  std::vector<PVector> previous_points;
  std::vector<PVector> current_points;
  float adjustedRadius = 0;

  // Set body color
  rdpq_set_prim_color(RED);

  // Draw joints
  for (size_t i = 0; i < spine.joints.size(); ++i) {
    if(i < 2){
      draw_ellipse(getPosX(i, 0, 0), getPosY(i, 0, 0), getBodyWidth(i), getBodyWidth(i));
    } else {
      adjustedRadius = getBodyWidth(i) - (i * 0.1f); // Taper width after second joint
      draw_ellipse(getPosX(i, 0, 0), getPosY(i, 0, 0), adjustedRadius, adjustedRadius);
    }

    get_ellipse_points(getPosX(i, 0, 0), getPosY(i, 0, 0), getBodyWidth(i), getBodyWidth(i), 14, current_points);

    if (!previous_points.empty()) {

      // Calculate centers for previous and current points
      PVector prev_center = {0, 0};
      PVector curr_center = {0, 0};
      for (const auto& p : previous_points) {
          prev_center.x += p.x;
          prev_center.y += p.y;
      }
      for (const auto& p : current_points) {
          curr_center.x += p.x;
          curr_center.y += p.y;
      }
      prev_center.x /= previous_points.size();
      prev_center.y /= previous_points.size();
      curr_center.x /= current_points.size();
      curr_center.y /= current_points.size();
      
      // Scale points outward to fill in any gaps
      e = 0.2f;
      float scale = 1.0f + e;
      for (int j = 0; j < 14; ++j) {
        PVector v1r = PVector::scale(prev_center, previous_points[j], scale);
        PVector v2r = PVector::scale(prev_center, previous_points[j + 1], scale);
        PVector v3r = PVector::scale(curr_center, current_points[j], scale);
        PVector v4r = PVector::scale(curr_center, current_points[j + 1], scale);

        // Create triangles between scaled points
        float v1f[] = { v1r.x, v1r.y };
        float v2f[] = { v2r.x, v2r.y };
        float v3f[] = { v3r.x, v3r.y };
        float v4f[] = { v4r.x, v4r.y };

        // Draw two triangles to form a quad between the points
        rdpq_triangle(&TRIFMT_FILL, v1f, v2f, v3f);
        rdpq_triangle(&TRIFMT_FILL, v2f, v4f, v3f);
        TC += 2;
      }
    }

    previous_points = current_points; // Save current points for the next iteration
  }

  // Draw eye outline
  rdpq_set_prim_color(DARK_GREEN);
  draw_ellipse(getPosX(0, M_PI / 2, -1), getPosY(0, M_PI / 2, -1), 1.5, 1.5);
  draw_ellipse(getPosX(0, -M_PI / 2, -1), getPosY(0, -M_PI / 2, -1), 1.5, 1.5);

  // Draw eyes
  rdpq_set_prim_color(GREEN);
  draw_ellipse(getPosX(0, M_PI / 2, -1), getPosY(0, M_PI / 2, -1), 1, 1);
  draw_ellipse(getPosX(0, -M_PI / 2, -1), getPosY(0, -M_PI / 2, -1), 1, 1);

  // Free vertices after drawing
  free(vertices);
  current_points.clear();
  previous_points.clear();

  //debugf("%u\n", TC);
  //TC = 0;
}
