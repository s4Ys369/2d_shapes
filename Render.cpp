#include <libdragon.h>
#include "Point.h"
#include "Shape.h"
#include "Render.h"
#include "Utils.h"

void Render::set_fill_color(color_t color){
    rdpq_set_prim_color(color);
}

// Function to get points around an ellipse
std::vector<Point> Render::get_ellipse_points(Point center, float rx, float ry, int segments) {
  std::vector<Point> points;
  float angleStep = 2 * M_PI / segments;
  for (int i = 0; i < segments; ++i) {
    float angle = i * angleStep;
    float x = center.x + rx * cosf(angle);
    float y = center.y + ry * sinf(angle);
    points.push_back(Point(x, y));
  }
  //debugf("Ellipse points: ");
  for (const auto& point : points) {
    //debugf("(%f, %f) ", point.x, point.y);
  }
  //debugf("\n");
  return points;
}

// Function to draw a triangle fan
void Render::draw_fan(const std::vector<Point>& points) {
  if (points.size() < 3){ debugf("Need at least 3 points to form a triangle"); return; }

  // First point is the center of the fan
  Point center = points[0];

  for (size_t i = 1; i < points.size() - 1; ++i) {
    Point v1 = center;
    Point v2 = points[i];
    Point v3 = points[i + 1];

    rdpq_triangle(&TRIFMT_FILL, &v1.x, &v2.x, &v3.x);
    triCount++;
    vertCount += 2;
  }

  //debugf("Drawing fan with points: ");
  for (const auto& point : points) {
    //debugf("(%f, %f) ", point.x, point.y);
  }
  //debugf("\n");
}

// Draw a uniformed closed shape of any number of vertices as a triangle fan
void Render::draw_ellipse(float cx, float cy, float rx, float ry, float angle, float lod) {

  /*
    Segments directly related to the number of triangles to be drawn.

    1 segment will draw a triangle with all vertices equal, so invisible.

    2 segments will draw 2 triangles with all vertices' Y positions equal,
    so again imperceptible. 

    3 segments with draw 3 triangles radiating from the center,
    effectively drawing one triangle wastefully.

    4 segments with draw 4 triangles radiating from the center,
    effectively drawing one rectangle wastefully.

    5 segments with draw 5 triangles radiating from the center,
    effectively drawing one pentagon, making 5 segments
    the best canditate for the minimum required.

    There may be some usecases for drawing triangles and rectangles,
    because all of the transformations (ie rotation, scale, etc.)
    can still be applied.

    Until said transformations are readily available for
    RDPQ tris and rects, keep the minimum amount of segments
    at 3.

  */

  int base_segments = 100; // Base number of segments for the highest LOD
  int segments = std::max(static_cast<int>(base_segments * lod), 3);

  float theta = 2.0f * M_PI / float(segments);

  // Calculate angles for position
  float cos_theta = fm_cosf(theta);
  float sin_theta = fm_sinf(theta);

  // Calculate angles for rotation
  float cos_angle = fm_cosf(angle);
  float sin_angle = fm_sinf(angle);

  std::vector<float> vertices;
  std::vector<int> indices;

  // Center vertex
  vertices.push_back(cx);
  vertices.push_back(cy);

  // Calculate perimeter vertices
  float x = rx;
  float y = 0.0f;

  // For segment/triangle
  for (int i = 0; i < segments; ++i) {

    // Apply rotation
    float rotatedX = x * cos_angle - y * sin_angle;
    float rotatedY = x * sin_angle + y * cos_angle;
    vertices.push_back(cx + rotatedX);
    vertices.push_back(cy + rotatedY);

    // Calculate next position from rotation matrix
    float nextX = cos_theta * x - sin_theta * y;
    float nextY = sin_theta * x + cos_theta * y;

    // Set position for next iteration
    x = nextX;
    y = nextY;
  
  }

  // Create indices for a triangle fan
  for (int i = 1; i <= segments; ++i) {
      indices.push_back(0);         // Center vertex, always first index for a fan
      indices.push_back(i);         // Current perimeter vertex
      indices.push_back((i % segments) + 1); // Next perimeter vertex
  }

  // Draw the indexed vertices
  rdpq_draw_indexed_triangles(&vertices[0], vertices.size(), &indices[0], indices.size());

}

// Function to draw a quad/rectangle of certain thickness with rotation and scale, using a 2 triangle strip
void Render::draw_line(float x1, float y1, float x2, float y2, float angle, float thickness) {
    // Define points
    Point start(x1, y1);
    Point end(x2, y2);

    // Calculate the center of the line segment
    Point center((x1 + x2) / 2.0f, (y1 + y2) / 2.0f);

    // Calculate direction vector
    Point direction = Point::sub(end, start);
    float length = direction.magnitude();

    // Check for non-zero length and normalize
    if (length > 0) {
        direction.normalize(); // Normalize the direction vector
    } else {
        debugf("Line length cannot be 0");
        return;
    }

    // Calculate the perpendicular vector for the thickness
    Point perp(-direction.y, direction.x); // Perpendicular to direction
    perp.set_mag(thickness / 2); // Set the magnitude to half of the thickness

    // Rotation matrix
    float cos_angle = fm_cosf(angle);
    float sin_angle = fm_sinf(angle);

    // Compute the points for the line
    Point p1_left = { start.x , start.y - perp.y };
    Point p1_right = { end.x , start.y - perp.y };
    Point p2_left = { start.x , end.y};
    Point p2_right = end;

    // Rotate each vertex around the center of the line segment
    auto rotate_line_point = [cos_angle, sin_angle](Point& p, const Point& center) {
        float tx = p.x - center.x;
        float ty = p.y - center.y;
        p.x = center.x + (tx * cos_angle - ty * sin_angle);
        p.y = center.y + (tx * sin_angle + ty * cos_angle);
    };

    // Rotate the trapezoid vertices
    rotate_line_point(p1_left, center);
    rotate_line_point(p1_right, center);
    rotate_line_point(p2_left, center);
    rotate_line_point(p2_right, center);

    // Define vertices for two triangles to form the line with thickness
    float v1[] = { p1_left.x, p1_left.y };
    float v2[] = { p1_right.x, p1_right.y };
    float v3[] = { p2_left.x, p2_left.y };
    float v4[] = { p2_right.x, p2_right.y };

    // Draw two triangles to form the line
    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3); // First triangle
    rdpq_triangle(&TRIFMT_FILL, v2, v4, v3); // Second triangle
    triCount += 2; // Increment triangle count
    vertCount += 4; // Increment vertex count
}


// Function to draw a Bézier curve as a triangle strip and returns the curve as a point array
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

    draw_line(p1.x, p1.y, p2.x, p2.y, 1.0f, thickness);
  }

}


// Function to fill area between 2 Bézier curves using quads/rectangles
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
      vertCount += 4; // Increment vertex count
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
    vertCount += 2; // Increment vertex count
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
  draw_fan(bottomPoints);

  bottomPoints.clear();
  topPoints.clear();
}

// Function to draw a quad/rectangle from the edge of an ellipse/fan to the edge of a "line" (ie another quad/rectangle)
void Render::fill_edge_ellipse_to_line(const std::vector<Point>& currentPoints, int segments, float scale) {
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
            vertCount += 4; // Increment vertex count
        }
    }

    previousPoints = currentPoints; // Save current points for the next iteration
}

