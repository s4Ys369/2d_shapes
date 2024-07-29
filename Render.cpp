#include <libdragon.h>
#include "Point.h"
#include "Shape.h"
#include "Render.h"
#include "Utils.h"

// Function to draw an ellipse and store points around the perimeter
void draw_ellipse(float cx, float cy, float rx, float ry, int segments, std::vector<Point>& points) {
    points.clear(); // Clear the vector to store new points

    // Calculate angles
    float theta = 2.0f * M_PI / float(segments);
    float cos_theta = fm_cosf(theta);
    float sin_theta = fm_sinf(theta);

    // Set width and zero height
    float x = rx;
    float y = 0;

    for (int i = 0; i < segments; ++i) {

        // Return the current position
        float currX = rx * cos_theta;
        float currX = ry * sin_theta;
        points.push_back({cx + currX, cy + currX});

        // Calculate next position
        float nextX = cos_theta * x - sin_theta * y;
        float nextY = sin_theta * x + cos_theta * y;

        // Set triangle vertices
        float v1[] = { cx, cy };
        float v2[] = { cx + x, cy + y };
        float v3[] = { cx + nextX, cy + nextY };

        // Draw =]
        rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
        triCount++;

        // Set position for next iteration
        x = nextX;
        y = nextY;
    }
}


// Function to draw the curved triangle fan
void draw_fan_curved(const std::vector<Point>& points) {
    if (points.size() < 3) debugf("Need at least 3 points to form a triangle"); return;

    // First point is the center of the fan
    Point center = points[0];

    for (size_t i = 1; i < points.size() - 1; ++i) {
        Point v1 = center;
        Point v2 = points[i];
        Point v3 = points[i + 1];

        rdpq_triangle(&TRIFMT_FILL, &v1.x, &v2.x, &v3.x);
        triCount++;
    }
}

// Function to draw a line segment of certain thickness using two triangles
void draw_line(float x1, float y1, float x2, float y2, float thickness) {
    // Calculate direction vector of the line
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);

    // Normalize the direction vector
    if (length != 0) {
      dx /= length;
      dy /= length;
    } else {
        debugf("Line length can not be 0");
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
void draw_bezier_curve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments, float thickness) {
  std::vector<Point> curvePoints;

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

    Point p = { uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
                  uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y };

    curvePoints.push_back(p);
  }

  // Draw the Bézier curve as a series of connected line segments
  for (size_t i = 0; i < curvePoints.size() - 1; ++i) {
    Point p1 = curvePoints[i];
    Point p2 = curvePoints[i + 1];

    draw_line(p1.x, p1.y, p2.x, p2.y , thickness);
  }

}


// Function to fill area between 2 Bézier curves using triangles
void fill_between_beziers(const std::vector<Point>& curve1, const std::vector<Point>& curve2) {
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
void draw_filled_beziers(const std::vector<Point>& upper_curve, const std::vector<Point>& lower_curve, int segments) {

    // Generate points for the upper Bézier curve
    for (int i = 0; i <= segments; ++i) {
        float t = float(i) / float(segments);
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        Point p = { uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
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

        Point q = { uuu * q0.x + 3 * uu * t * q1.x + 3 * u * tt * q2.x + ttt * q3.x,
                      uuu * q0.y + 3 * uu * t * q1.y + 3 * u * tt * q2.y + ttt * q3.y };
        lower_curve.push_back(q);
    }

    // Fill the area between the two curves
    fill_between_beziers(lower_curve, upper_curve);
}

// Function to check ear clipping, An "ear" is a triangle formed by three consecutive vertices in a polygon that does not contain any other vertices of the polygon inside it.
bool is_ear(const std::vector<Point>& polygon, int u, int v, int w, const std::vector<int>& V) {
  const Point& A = polygon[V[u]];
  const Point& B = polygon[V[v]];
  const Point& C = polygon[V[w]];

  if (Point::epsilon_test(A, B, C) >= 0) {
    return false;
  }

  for (size_t p = 0; p < polygon.size(); ++p) {
    if ((p == static_cast<size_t>(u)) || (p == static_cast<size_t>(v)) || (p == static_cast<size_t>(w))) {
      continue;
    }
    if (Point::point_in_triangle(polygon[p], A, B, C)) {
      return false;
    }
  }

  return true;
}

// A simple ear clipping algorithm for triangulation
void triangulate_polygon(const std::vector<Point>& polygon, std::vector<Point>& triangles) {
  std::vector<int> V(polygon.size());
  for (size_t i = 0; i < polygon.size(); ++i) {
    V[i] = i;
  }

  int n = polygon.size();
  int count = 2 * n;

  for (int v = n - 1; n > 2;) {
    if ((count--) <= 0) {
      debugf("No polygon detected\n");
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
void draw_filled_bezier_shape(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments) {
  std::vector<Point> curvePoints;

  // Compute Bézier curve points
  for (int i = 0; i <= segments; ++i) {
    float t = float(i) / float(segments);
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    Point p = { uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
                  uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y };

    curvePoints.push_back(p);
  }

  // Close the polygon by connecting the last point back to the first
  curvePoints.push_back(curvePoints[0]);

  // Triangulate the closed polygon (using a simple ear clipping method)
  std::vector<Point> triangles;
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

// Function to draw triangle fan by transforming points based on width
void draw_fan_transform(const std::vector<Point>& point, const std::vector<float>& p2, int segments, float width) {
  std::vector<Point> bottomPoints;
  std::vector<Point> topPoints;
  int midPoint = fm_ceilf(segments * (2.0f/3.0f));
  int loopPoint = segments - 1;

    // Bottom of fan
    for (int i = midPoint; i < segments; ++i) {
        Point transformedPoint = Point::transform(point[i], a[i] - M_PI / 2, width);
        bottomPoints.push_back(transformedPoint);
    }

    // Top of fan
    for (int i = loopPoint; i >= midPoint; --i) {
        Point transformedPoint = Point::transform(point[i], a[i] + M_PI / 2, width);
        topPoints.push_back(transformedPoint);
    }

    // Draw the bottom and top of the fan as a single closed shape
    bottomPoints.insert(bottomPoints.end(), topPoints.begin(), topPoints.end());
    draw_fan_curved(bottomPoints);

    bottomPoints.clear();
    topPoints.clear();
}

// Function to fill point
void fill_edge_to_ellipse(const std::vector<Point>& currentPoints, int segments, float scale) {
    static std::vector<Point> previousPoints;  // Static to persist between function calls

    if (!previousPoints.empty()) {

        // Calculate centers for previous and current points
        Point prevCenter = {0, 0};
        Point currCenter = {0, 0};
        for (const auto& p : previousPoints) {
            prevCenter.x += p.x;
            prevCenter.y += p.y; 
        }
        for (const auto& p : currentPoints) {
            currCenter.x += p.x;
            currCenter.y += p.y;
        }
        prevCenter.x /= previousPoints.size();
        prevCenter.y /= previousPoints.size();
        currCenter.x /= currentPoints.size();
        currCenter.y /= currentPoints.size();

        // Scale points outward to fill in any gaps
        for (int i = 0; i < segments; ++i) {
            Point v1r = Point::scale(prevCenter, previousPoints[i], scale);
            Point v2r = Point::scale(prevCenter, previousPoints[(i + 1) % segments], scale); // Use modulo to wrap around
            Point v3r = Point::scale(currCenter, currentPoints[i], scale);
            Point v4r = Point::scale(currCenter, currentPoints[(i + 1) % segments], scale); // Use modulo to wrap around

            // Create triangles between scaled points
            float v1f[] = { v1r.x, v1r.y };
            float v2f[] = { v2r.x, v2r.y };
            float v3f[] = { v3r.x, v3r.y };
            float v4f[] = { v4r.x, v4r.y };

            // Draw two triangles to form a quad between the points
            rdpq_triangle(&TRIFMT_FILL, v1f, v2f, v3f);
            rdpq_triangle(&TRIFMT_FILL, v2f, v4f, v3f);
            triCount++;
        }
    }

    previousPoints = currentPoints; // Save current points for the next iteration
}

// Function to draw fill shape by defining edges and interior ellipses, then filling the gaps
void draw_filled_shape() {
  size_t vertex_count = 0;
  size_t max_vertices = spine.joints.size() + 4; // Adjust this as needed
  Point* vertices = (Point*)malloc(max_vertices * sizeof(Point));
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
    // Cast Point to float[] for `rdpq_triangle`
    float v1[] = { vertices[i].x, vertices[i].y };
    float v2[] = { vertices[i + 1].x, vertices[i + 1].y };
    float v3[] = { vertices[i + 2].x, vertices[i + 2].y };

    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    triCount++;
  }

  std::vector<Point> previous_points;
  std::vector<Point> current_points;
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
      Point prev_center = {0, 0};
      Point curr_center = {0, 0};
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
        Point v1r = Point::scale(prev_center, previous_points[j], scale);
        Point v2r = Point::scale(prev_center, previous_points[j + 1], scale);
        Point v3r = Point::scale(curr_center, current_points[j], scale);
        Point v4r = Point::scale(curr_center, current_points[j + 1], scale);

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
  std::vector<Point> j = spine.joints;
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
  Point j4 = {j[4].x + cosf(a[4] + M_PI/2) * headToMid2 * 8,
                j[4].y + sinf(a[4] + M_PI/2) * headToMid2 * 8,};
  Point j3 = {j[3].x + cosf(a[3] + M_PI/2) * headToMid2 * 8,
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

