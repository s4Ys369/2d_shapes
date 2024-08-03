#include <libdragon.h>
#include "Chain.h"
#include "Fish.h"
#include "Utils.h"

int triCount = 0;

Fish::Fish(PVector origin)
  : spine(origin, 12, 6, M_PI / 8),
    bodyColor(BLUE),
    finColor(RED),
    bodyWidth{3.2f, 7.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f ,1.0f} {}

void Fish::resolve(float mouseX, float mouseY) {
  PVector headPos = spine.joints[0];
  PVector targetPos = headPos;

  // Apply deadzone to the joystick inputs
  float adjustedX = apply_deadzone(mouseX);
  float adjustedY = apply_deadzone(mouseY);

  // Normalize the joystick input to get the direction
  PVector direction(adjustedX, -adjustedY);
  direction.normalize();

  // Limit fish movement to inside screen with offset
  float offset = 32.0f;
  float width = display_get_width();
  float height = display_get_height();
  PVector center = {160.0f,120.f};

  if(headPos.x < offset){headPos.x = offset; targetPos = center;}
  if(headPos.x > width - offset){headPos.x = width - offset; targetPos = center;}
  if(headPos.y < offset){headPos.y = offset; targetPos = center;}
  if(headPos.y > height - offset){headPos.y = height - offset; targetPos = center;}

  // Determine the target position based on the direction and a fixed magnitude
  float movementMag = 3.5;
  targetPos = PVector::add(headPos, direction.setMag(movementMag));

  spine.resolve(targetPos);
  //debugf("X %.1f\nY %.1f\n", targetPos.x, targetPos.y);
}

float Fish::getPosX(int i, float angleOffset, float lengthOffset) {
  return spine.joints[i].x + cosf(spine.angles[i] + angleOffset) * (bodyWidth[i] + lengthOffset);
}

float Fish::getPosY(int i, float angleOffset, float lengthOffset) {
  return spine.joints[i].y + sinf(spine.angles[i] + angleOffset) * (bodyWidth[i] + lengthOffset);
}

float Fish::getBodyWidth(int i) {
  return bodyWidth[i];
}

void Fish::draw_ellipse(float cx, float cy, float rx, float ry) {
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
    triCount++;

    x = next_x;
    y = next_y;
  }
}

// Function to get points around an ellipse
void Fish::get_ellipse_points(float cx, float cy, float rx, float ry, int segments, std::vector<PVector>& points) {
  points.clear(); // Clear the vector to store new points
  for (int i = 0; i <= segments; ++i) {
    float theta = 2.0f * M_PI * float(i) / float(segments);
    float x = rx * cosf(theta);
    float y = ry * sinf(theta);
    points.push_back({cx + x, cy + y});
  }
}

// Function to draw fin as ellipse at transformed position
void Fish::draw_fin(float posX, float posY, float rotation, float width, float height) {
  // Apply transformations
  PVector origin = {0,0};
  origin = PVector::translate(origin, posX, posY);
  origin = PVector::transform(origin, rotation, width);

  // Draw the ellipse at the transformed position
  //debugf("Drawing fin at: (%f, %f) with rotation: %f\n", origin.x, origin.y, rotation);
  draw_ellipse(origin.x, origin.y, width, height);
}

// Function to compute tail width
float Fish::get_tail_width(int i, float headToTail) {
  if (i < 12) {
    return 1.5f * headToTail * (i - 8) * (i - 8) / 4.0f;
  }
  return fmaxf(-13.0f, fminf(13.0f, headToTail));
}

// Function to draw the curve using a triangle fan
void Fish::draw_curve(const std::vector<PVector>& points) {
    if (points.size() < 3) return; // Need at least 3 points to form a triangle

    // First point is the center of the fan
    PVector center = points[0];

    for (size_t i = 1; i < points.size() - 1; ++i) {
        PVector v1 = center;
        PVector v2 = points[i];
        PVector v3 = points[i + 1];

        rdpq_triangle(&TRIFMT_FILL, &v1.x, &v2.x, &v3.x);
        triCount++;
    }
}

// Function to draw a line segment of certain thickness using two triangles
void Fish::draw_line(float x1, float y1, float x2, float y2, float thickness) {
    // Calculate direction vector of the line
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);

    // Normalize the direction vector
    if (length != 0) {
      dx /= length;
      dy /= length;
    }

    // Calculate the perpendicular vector for the thickness
    float perp_x = -dy * thickness / 2;
    float perp_y = dx * thickness / 2;

    // Define vertices of the trapezoid
    float x1_left = x1 + perp_x;
    float y1_left = y1 + perp_y;
    float x1_right = x1 - perp_x;
    float y1_right = y1 - perp_y;
    float x2_left = x2 + perp_x;
    float y2_left = y2 + perp_y;
    float x2_right = x2 - perp_x;
    float y2_right = y2 - perp_y;

    // Define vertices for two triangles
    float v1[] = { x1_left, y1_left };
    float v2[] = { x1_right, y1_right };
    float v3[] = { x2_left, y2_left };
    float v4[] = { x2_right, y2_right };

    // Draw two triangles to form the trapezoid
    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3); // First triangle
    rdpq_triangle(&TRIFMT_FILL, v2, v4, v3); // Second triangle
    triCount += 2;
}

// Function to draw a Bézier curve using line segments and returns the curve as a point 
void Fish::draw_bezier_curve(const PVector& p0, const PVector& p1, const PVector& p2, const PVector& p3, int segments) {
  std::vector<PVector> curvePoints;

  // Flush previous curve points
  curvePoints.clear();  

  // Compute Bézier curve points
  for (int i = 0; i <= segments; ++i) {
    float t = float(i) / float(segments);
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    PVector p = { uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
                  uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y };

    curvePoints.push_back(p);
  }

  // Draw the Bézier curve as a series of connected line segments
  for (size_t i = 0; i < curvePoints.size() - 1; ++i) {
    PVector p1 = curvePoints[i];
    PVector p2 = curvePoints[i + 1];

    draw_line(p1.x, p1.y, p2.x, p2.y , 1.0f);
  }

}


// Function to fill area between 2 Bézier curves using triangles
void Fish::fill_between_beziers(const std::vector<PVector>& curve1, const std::vector<PVector>& curve2) {
  size_t size = std::min(curve1.size(), curve2.size());
  for (size_t i = 0; i < size - 1; ++i) {
      float v1[] = { curve1[i].x, curve1[i].y };
      float v2[] = { curve1[i + 1].x, curve1[i + 1].y };
      float v3[] = { curve2[i].x, curve2[i].y };
      float v4[] = { curve2[i + 1].x, curve2[i + 1].y };

      // Draw two triangles to fill the quad
      rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
      rdpq_triangle(&TRIFMT_FILL, v2, v3, v4);
      triCount += 2;
  }
}

// Function to draw a filled shape between 2 Bézier curves
void Fish::draw_filled_beziers(const PVector& p0, const PVector& p1, const PVector& p2, const PVector& p3, 
                               const PVector& q0, const PVector& q1, const PVector& q2, const PVector& q3, 
                               int segments) {
    std::vector<PVector> upper_curve;
    std::vector<PVector> lower_curve;

    // Generate points for the upper Bézier curve
    for (int i = 0; i <= segments; ++i) {
        float t = float(i) / float(segments);
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        PVector p = { uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
                      uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y };
        upper_curve.push_back(p);
    }

    // Generate points for the lower Bézier curve
    for (int i = 0; i <= segments; ++i) {
        float t = float(i) / float(segments);
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        PVector q = { uuu * q0.x + 3 * uu * t * q1.x + 3 * u * tt * q2.x + ttt * q3.x,
                      uuu * q0.y + 3 * uu * t * q1.y + 3 * u * tt * q2.y + ttt * q3.y };
        lower_curve.push_back(q);
    }

    // Fill the area between the two curves
    fill_between_beziers(lower_curve, upper_curve);
    lower_curve.clear();
    upper_curve.clear();
}

// Function to check ear clipping, An "ear" is a triangle formed by three consecutive vertices in a polygon that does not contain any other vertices of the polygon inside it.
bool Fish::is_ear(const std::vector<PVector>& polygon, int u, int v, int w, const std::vector<int>& V) {
  const PVector& A = polygon[V[u]];
  const PVector& B = polygon[V[v]];
  const PVector& C = polygon[V[w]];

  if (PVector::epsilon_test(A, B, C) >= 0) {
    return false;
  }

  for (size_t p = 0; p < polygon.size(); ++p) {
    if ((p == static_cast<size_t>(u)) || (p == static_cast<size_t>(v)) || (p == static_cast<size_t>(w))) {
      continue;
    }
    if (PVector::point_in_triangle(polygon[p], A, B, C)) {
      return false;
    }
  }

  return true;
}

// A simple ear clipping algorithm for triangulation
void Fish::triangulate_polygon(const std::vector<PVector>& polygon, std::vector<PVector>& triangles) {
  std::vector<int> V(polygon.size());
  for (size_t i = 0; i < polygon.size(); ++i) {
    V[i] = i;
  }

  int n = polygon.size();
  int count = 2 * n;

  for (int v = n - 1; n > 2;) {
    if ((count--) <= 0) {
      debugf("No polygon!\n");
      return;
    }

    int u = v;
    if (n <= u) {
      u = 0;
    }
    v = u + 1;
    if (n <= v) {
      v = 0;
    }
    int w = v + 1;
    if (n <= w) {
      w = 0;
    }

    int a = V[u], b = V[v], c = V[w];
    triangles.push_back(polygon[a]);
    triangles.push_back(polygon[b]);
    triangles.push_back(polygon[c]);

    for (int s = v, t = v + 1; t < n; s++, t++) {
      V[s] = V[t];
    }
    n--;

    count = 2 * n;
  }
}

// Function to draw a Bézier curve using line segments, then fill shape with triangles. Note the base will always be a straight line.
void Fish::draw_filled_bezier_shape(const PVector& p0, const PVector& p1, const PVector& p2, const PVector& p3, int segments) {
  std::vector<PVector> curvePoints;

  // Compute Bézier curve points
  for (int i = 0; i <= segments; ++i) {
    float t = float(i) / float(segments);
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    PVector p = { uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
                  uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y };

    curvePoints.push_back(p);
  }

  // Close the polygon by connecting the last point back to the first
  curvePoints.push_back(curvePoints[0]);

  // Triangulate the closed polygon (using a simple ear clipping method)
  std::vector<PVector> triangles;
  triangulate_polygon(curvePoints, triangles);

  // Draw the triangles
  for (size_t i = 0; i < triangles.size(); i += 3) {
    float v1[] = { triangles[i].x, triangles[i].y };
    float v2[] = { triangles[i + 1].x, triangles[i + 1].y };
    float v3[] = { triangles[i + 2].x, triangles[i + 2].y };

    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    triCount++;
  }

  curvePoints.clear();
  triangles.clear();
}

// Function to draw caudal fin by transforming points based on width
void Fish::draw_tail(const std::vector<PVector>& j, const std::vector<float>& a, float headToTail) {
  std::vector<PVector> bottomPoints;
  std::vector<PVector> topPoints;

  // "Bottom" of the fish
  for (int i = 8; i < 12; ++i) {
    float tailWidth = get_tail_width(i, headToTail);
    PVector transformedPoint = PVector::transform(j[i], a[i] - M_PI / 2, tailWidth);
    bottomPoints.push_back(transformedPoint);
  }

  // "Top" of the fish
  for (int i = 11; i >= 8; --i) {
    float tailWidth = get_tail_width(i, headToTail);
    PVector transformedPoint = PVector::transform(j[i], a[i] + M_PI / 2, tailWidth);
    topPoints.push_back(transformedPoint);
  }

  // Draw the "bottom" and "top" of the tail as a single closed shape
  bottomPoints.insert(bottomPoints.end(), topPoints.begin(), topPoints.end());
  draw_curve(bottomPoints); // Draw the shape

  bottomPoints.clear();
  topPoints.clear();
}


PVector* vertices;
void Fish::draw_body() {
  size_t vertex_count = 0;
  size_t max_vertices = spine.joints.size() + 4; // Adjust this as needed
  vertices = (PVector*)malloc(max_vertices * sizeof(PVector));
  //debugf("%u\n", max_vertices);
    
  if (!vertices) {
    debugf("No vertices!\n");
    return;
  }

  // Right half of the fish
  for (size_t i = 0; i < 10; i++) {
    vertices[vertex_count].x = getPosX(i, M_PI / 2, 0);
    vertices[vertex_count].y = getPosY(i, M_PI / 2, 0);
    vertex_count++;
    //debugf("%u\n", vertex_count);
  }

  // Top of the head (completes the loop)
  vertices[vertex_count].x = getPosX(9, M_PI, 0);
  vertices[vertex_count].y = getPosY(9, M_PI, 0);
  vertex_count++;
  //debugf("%u\n", vertex_count);

  // Left half of the fish
  for (int i = 9; i >= 0; --i) {
    vertices[vertex_count].x = getPosX(i, -M_PI / 2, 0);
    vertices[vertex_count].y = getPosY(i, -M_PI / 2, 0);
    vertex_count++;
    //debugf("%u\n", vertex_count);
  }

  // Add vertices to complete the loop
  vertices[vertex_count].x = getPosX(0, -M_PI / 6, 0);
  vertices[vertex_count].y = getPosY(0, -M_PI / 6, 0);
  vertex_count++;
  //debugf("%u\n", vertex_count);
  vertices[vertex_count].x = getPosX(0, 0, 0);
  vertices[vertex_count].y = getPosY(0, 0, 0);
  vertex_count++;
  //debugf("%u\n", vertex_count);
  vertices[vertex_count].x = getPosX(0, M_PI / 6, 0);
  vertices[vertex_count].y = getPosY(0, M_PI / 6, 0);
  vertex_count++;
  //debugf("%u\n", vertex_count);

  // Draw edges
  for (size_t i = 0; i < vertex_count - 2; ++i) {
    // Cast PVector to float[] for `rdpq_triangle`
    float v1[] = { vertices[i].x, vertices[i].y };
    float v2[] = { vertices[i + 1].x, vertices[i + 1].y };
    float v3[] = { vertices[i + 2].x, vertices[i + 2].y };

    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    triCount++;
  }

  std::vector<PVector> previous_points;
  std::vector<PVector> current_points;
  float adjustedRadius = 0;

  // Draw joints
  for (size_t i = 0; i < spine.joints.size() - 2; ++i) {
    if (i < 2) {
      draw_ellipse(getPosX(i, 0, 0), getPosY(i, 0, 0), getBodyWidth(i), getBodyWidth(i));
    } else { // Don't draw the last two tail joints
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
      float e = 0.1f;
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
        triCount += 2;
      }
    }

    

    previous_points = current_points; // Save current points for the next iteration
  }
  free(vertices);
  current_points.clear();
  previous_points.clear();
}

void Fish::display() {
  rdpq_sync_pipe();

  rdpq_set_prim_color(finColor);

  // Alternate labels for shorter lines of code
  std::vector<PVector> j = spine.joints;
  std::vector<float> a = spine.angles;

  // Relative angle differences are used in some hacky computation for the dorsal fin
  float headToMid1 = relativeAngleDiff(a[0], a[4]);
  float headToMid2 = relativeAngleDiff(a[0], a[5]);

  // For the caudal fin, we need to compute the relative angle difference from the head to the tail, but given
  // a joint count of 12 and angle constraint of PI/8, the maximum difference between head and tail is 11PI/8,
  // which is >PI. This complicates the relative angle calculation (flips the sign when curving too tightly).
  // A quick workaround is to compute the angle difference from the head to the middle of the fish, and then
  // from the middle of the fish to the tail.
  float headToTail = headToMid1 + relativeAngleDiff(a[6], a[11]);

  // === START CAUDAL FINS ===

  // Draw the tail
  draw_tail(j, a, headToTail);

  // === END CAUDAL FINS ===

  // === START PECTORAL FINS ===

  // Drawing the right fin
  float xPosRight_pec = getPosX(4, M_PI / 3, 0);
  float yPosRight_pec = getPosY(4, M_PI / 3, 0);
  float angleRight_pec = a[3] - M_PI / 4;
  //debugf("Right fin position: (%f, %f), angle: %f\n", xPosRight_pec, yPosRight_pec, angleRight_pec);
  draw_fin(xPosRight_pec, yPosRight_pec, angleRight_pec, 10.0f, 3.0f);

  // Drawing the left fin
  float xPosLeft_pec = getPosX(4, -M_PI / 3, 0);
  float yPosLeft_pec = getPosY(4, -M_PI / 3, 0);
  float angleLeft_pec = a[3] + M_PI / 4;
  draw_fin(xPosLeft_pec, yPosLeft_pec, angleLeft_pec, 10.0f, 3.0f);
  // === END PECTORAL FINS ===

  // === START VENTRAL FINS ===

  // Drawing the right fin
  float xPosRight_ven = getPosX(7, M_PI / 2, 0);
  float yPosRight_ven = getPosY(7, M_PI / 2, 0);
  float angleRight_ven = a[6] - M_PI / 4;
  draw_fin(xPosRight_ven, yPosRight_ven, angleRight_ven, 6.0f, 2.0f);

  // Drawing the left fin
  float xPosLeft_ven = getPosX(7, -M_PI / 2, 0);
  float yPosLeft_ven = getPosY(7, -M_PI / 2, 0);
  float angleLeft_ven = a[6] + M_PI / 4;
  draw_fin(xPosLeft_ven, yPosLeft_ven, angleLeft_ven, 6.0f, 2.0f);

  // === END VENTRAL FINS ===


  // === START BODY ===
  
  rdpq_set_prim_color(bodyColor);

  // Draw the body
  draw_body();

  // === END BODY ===


  // === START DORSAL FIN ===

  rdpq_set_prim_color(finColor);

  //Transform the points for the outside
  PVector j4 = {j[4].x + cosf(a[4] + M_PI/2) * headToMid2 * 8,
                j[4].y + sinf(a[4] + M_PI/2) * headToMid2 * 8,};
  PVector j3 = {j[3].x + cosf(a[3] + M_PI/2) * headToMid2 * 8,
                j[3].y + sinf(a[3] + M_PI/2) * headToMid2 * 8,};

  draw_filled_beziers(
    j[5], j4, j3, j[2], // upper curve
    j[2], j[3], j[4], j[5], // lower curve
    7 // segments for each
  );

  /*
    The draw filled beziers function always seems to overdraw the lower curve,
    so draw another shape with the body color on top of the first to achieve
    desired visual effect.
  */
  rdpq_set_prim_color(bodyColor);
  draw_filled_beziers(
    j[5], j[4], j[3], j[2],
    j[2], j[3], j[4], j[5],
    5
  );

  // === END DORSAL FIN ===

  // === START EYES ===
  
  // Draw eye outline
  rdpq_set_prim_color(DARK_GREEN);
  draw_ellipse(getPosX(0, M_PI / 2, -1), getPosY(0, M_PI / 2, -1), 1.5, 1.5);
  draw_ellipse(getPosX(0, -M_PI / 2, -1), getPosY(0, -M_PI / 2, -1), 1.5, 1.5);

  // Draw eyes
  rdpq_set_prim_color(GREEN);
  draw_ellipse(getPosX(0, M_PI / 2, -1), getPosY(0, M_PI / 2, -1), 1, 1);
  draw_ellipse(getPosX(0, -M_PI / 2, -1), getPosY(0, -M_PI / 2, -1), 1, 1);

  // === END EYES ===

  j.clear();
  a.clear();

  //debugf("%u\n", triCount);
  //triCount = 0;
}

