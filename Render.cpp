#include <libdragon.h>
#include "Point.h"
#include "Shape.h"
#include "Render.h"
#include "Utils.h"

void Render::set_fill_color(color_t color){
    rdpq_set_prim_color(color);
}

// Function to get points around an ellipse
void Render::get_ellipse_points(float cx, float cy, float rx, float ry, int segments, std::vector<Point>& points) {
    points.clear();
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = rx * cosf(theta);
        float y = ry * sinf(theta);
        points.push_back({cx + x, cy + y});
    }
}

// Function to draw an ellipse and store points around the perimeter
void Render::draw_ellipse(float cx, float cy, float rx, float ry, int segments) {
    // Calculate angles
    float theta = 2.0f * M_PI / float(segments);
    float cos_theta = fm_cosf(theta);
    float sin_theta = fm_sinf(theta);

    // Set initial positions
    float x = rx;
    float y = 0.0f;

    for (int i = 0; i < segments; ++i) {

        // Calculate the next position using rotation matrix
        float nextX = cos_theta * x - sin_theta * y;
        float nextY = sin_theta * x + cos_theta * y;

        float v1[] = { cx, cy };
        float v2[] = { cx + x, cy + y };
        float v3[] = { cx + nextX, cy + nextY };

        // Draw the triangle
        rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
        triCount++;

        // Set position for next iteration
        x = nextX;
        y = nextY;
    }
}


// Function to draw the curved triangle fan
void Render::draw_fan_curved(const std::vector<Point>& points) {
    if (points.size() < 3){ debugf("Need at least 3 points to form a triangle"); return; }

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
void Render::draw_line(float x1, float y1, float x2, float y2, float thickness) {
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
void Render::draw_bezier_curve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments, float thickness) {
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
void Render::fill_between_beziers(const std::vector<Point>& curve1, const std::vector<Point>& curve2) {
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
void Render::draw_filled_beziers(const Point& p0, const Point& p1, const Point& p2, const Point& p3, 
                               const Point& q0, const Point& q1, const Point& q2, const Point& q3, 
                               int segments) {
    std::vector<Point> upper_curve;
    std::vector<Point> lower_curve;

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
    lower_curve.clear();
    upper_curve.clear();
}

// Function to check ear clipping, An "ear" is a triangle formed by three consecutive vertices in a polygon that does not contain any other vertices of the polygon inside it.
bool Render::is_ear(const std::vector<Point>& polygon, int u, int v, int w, const std::vector<int>& V) {
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
void Render::triangulate_polygon(const std::vector<Point>& polygon, std::vector<Point>& triangles) {
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
void Render::draw_filled_bezier_shape(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments) {
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
void Render::draw_fan_transform(const std::vector<Point>& point, const std::vector<float>& angle, int segments, float width) {
  std::vector<Point> bottomPoints;
  std::vector<Point> topPoints;
  int midPoint = fm_ceilf(segments * (2.0f/3.0f));
  int loopPoint = segments - 1;

    // Bottom of fan
    for (int i = midPoint; i < segments; ++i) {
        Point transformedPoint = Point::transform(point[i], angle[i] - M_PI / 2, width);
        bottomPoints.push_back(transformedPoint);
    }

    // Top of fan
    for (int i = loopPoint; i >= midPoint; --i) {
        Point transformedPoint = Point::transform(point[i], angle[i] + M_PI / 2, width);
        topPoints.push_back(transformedPoint);
    }

    // Draw the bottom and top of the fan as a single closed shape
    bottomPoints.insert(bottomPoints.end(), topPoints.begin(), topPoints.end());
    draw_fan_curved(bottomPoints);

    bottomPoints.clear();
    topPoints.clear();
}

// Function to fill point
void Render::fill_edge_to_ellipse(const std::vector<Point>& currentPoints, int segments, float scale) {
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

