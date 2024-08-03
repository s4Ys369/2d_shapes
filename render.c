#include <libdragon.h>
#include "point.h"
#include "shapes.h"
#include "render.h"
#include "utils.h"

void set_render_color(color_t color){
  rdpq_set_prim_color(color);
}

void render_move_point(PointArray* pa, size_t index, float dx, float dy) {
  if (index < pa->count) {
    point_add_in_place(&pa->points[index], point_new(dx,dy));
  }
}
 
void render_move_shape_points(PointArray* pa, float dx, float dy) {
  for (int i = 0; i < pa->count; ++i) {
    point_add_in_place(&pa->points[i], &direction);
  }
}

void render_rotate_point(PointArray* pa, size_t index, Point center, float angle) {
  if (index < pa->count) {
    point_rotate(&pa->points[index], center, angle)
  }
}

void render_rotate_shape_points(PointArray* pa, Point center, float angle) {
  for (int i = 0; i < pa->count; ++i) {
    point_add_in_place(&pa->points[i], &direction);
  }
}

// Function to get points around an ellipse
PointArray render_get_ellipse_points(Point center, float rx, float ry, int segments) {

  if(segments == 0){
    segments = 1;
  }

  PointArray pa;
  pa.points = (Point*)malloc(sizeof(Point) * segments);
  pa.count = segments;

  float angleStep = 2 * M_PI / segments;
  for (int i = 0; i < segments; ++i) {
    float angle = i * angleStep;
    float x = center.x + rx * fm_cosf(angle);
    float y = center.y + ry * fm_sinf(angle);
    pa.points[i] = (Point){x, y};
  }

  return points;
}

// Texture test
void draw_triangle(float* v1, float* v2, float* v3) {

  //const rdpq_trifmt_t trifmt = (rdpq_trifmt_t){
  //  .pos_offset = 0,
  //  .tex_offset = 3,
  //  .tex_tile = TILE0,
  //};

  float A[] = {v1[0],v1[1],0,0,1};
  float B[] = {v2[0],v2[1],0,0,1};
  float C[] = {v3[0],v3[1],0,0,1};

        
  rdpq_triangle(&TRIFMT_TEX, A, B, C);

}

// Function to draw RDPQ triangles using vertex arrays
void draw_indexed_triangles(float* vertices, int vertex_count, int* indices, int index_count) {
  for (int i = 0; i < index_count; i += 3) {
    int idx1 = indices[i];
    int idx2 = indices[i + 1];
    int idx3 = indices[i + 2];
        
    float v1[] = { vertices[idx1 * 2], vertices[idx1 * 2 + 1] };
    float v2[] = { vertices[idx2 * 2], vertices[idx2 * 2 + 1] };
    float v3[] = { vertices[idx3 * 2], vertices[idx3 * 2 + 1] };
        
    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    triCount++;
    vertCount++;
  }
}

// Function to draw a triangle fan from an array of points
void draw_fan(const PointArray* points) {
  if (points->count < 3){ debugf("Need at least 3 points to form a triangle"); return; }

  // First point is the center of the fan
  Point center = points[0];

  for (size_t i = 1; i < points->count - 1; ++i) {
    Point v1 = center;
    Point v2 = points[i];
    Point v3 = points[i + 1];

    rdpq_triangle(&TRIFMT_FILL, &v1.x, &v2.x, &v3.x);
    triCount++;
    vertCount++;
  }

}

// Draw a uniformed circle of any number of vertices as a triangle fan
void draw_circle(float cx, float cy, float rx, float ry, float angle, float lod) {

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
  int segments = (int)fmaxf((float)base_segments * lod, 3.0f);

  // Area thresholds
  float area = rx * 2.0f;
  float offset = (float)area * 0.3f;

  //debugf("Area %.0f\n", area);

  if(area <= 0.9f) {

    // If only drawing subpixels, exit
    debugf("Do you really need subpixels?\n");
    return;

  } else if (area >= 1.0f && area < 2.9f) {

    // If only drawing ~4 pixels or less, just draw a quad to save triangles
    draw_line(cx-offset,cy-offset,cx+offset,cy+offset,angle,1.0f);
    //debugf("Simpilifed to quad\n");
    return;

  } else  if (area >= 3.0f && area <= 4.9f) {

    segments = (int)(area)/2;
    if(segments < 6){
      segments = 6;
    }
    //debugf("Segments %u\n", segments);

  } else  if (area > 5.0f && area <= 9.9f) {

    segments = (int)(area)/3;
  
    if(segments < 6){
      segments = 6;
    }

    //debugf("Segments %u\n", segments);

  } else  if (area > 9.9f) {

    segments = (int)(area)/2;

    if(segments < 6){
      segments = 6;
    }

    // CHANGE: Set highest level of detail, currently 100 triangles
    if(segments > 200 || lod > 2.0f){
      segments = 200;
    }

    //debugf("Segments %u\n", segments);
  }

  // Calculate angles for position
  float theta = 2.0f * M_PI / float(segments);
  float cos_theta = fm_cosf(theta);
  float sin_theta = fm_sinf(theta);

  // Calculate angles for rotation
  float cos_angle = fm_cosf(angle);
  float sin_angle = fm_sinf(angle);

  // Initialize vert arrays
  float* vertices = NULL;
  int vertex_count = (segments + 1) * 2; // fan uses 1 center vert and a vert for ever new segments
  add_vertex(&vertices, &vertex_count, cx, cy);

  // Calculate perimeter vertices
  float x = rx;
  float y = 0.0f;

  // Per segment/triangle
  for (int i = 0; i < segments; ++i) {

    // Apply rotation
    float rotatedX = x * cos_angle - y * sin_angle;
    float rotatedY = x * sin_angle + y * cos_angle;
    add_vertex(&vertices, &vertex_count, cx + rotatedX, cy+ rotatedY);

    // Calculate next position from rotation matrix
    float nextX = cos_theta * x - sin_theta * y;
    float nextY = sin_theta * x + cos_theta * y;

    // Set position for next iteration
    x = nextX;
    y = nextY;
  
  }

  // Create indices for a triangle fan
  int* indices = create_triangle_fan_indices(segments, &index_count);

  // Draw the indexed vertices
  draw_indexed_triangles(&vertices[0], vertices.size(), &indices[0], index_count);

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


// Function to draw a Bézier curve as a triangle strip with a given thickness
void Render::draw_bezier_curve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments, float angle, float thickness) {
  std::vector<Point> curvePoints;
  std::vector<float> vertices;
  std::vector<int> indices;

  curvePoints.clear();
  curvePoints.reserve(segments + 1);

  float step = 1.0f / float(segments);

  // Compute Bézier curve points
  for (int i = 0; i <= segments; ++i) {
    float t = i * step;
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    float x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
    float y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;

    curvePoints.emplace_back(Point{x, y});
  }

  // Center of the curve for rotation
  Point center = { (p0.x + p3.x) / 2.0f, (p0.y + p3.y) / 2.0f };

  // Apply rotation to all curve points
  for (auto& p : curvePoints) {
    p.rotate(center,angle);
  }

  // Create left and right side points for the curve
  for (size_t i = 0; i < curvePoints.size(); ++i) {
    Point p = curvePoints[i];

    // Calculate the direction vector
    Point dir;
    if (i < curvePoints.size() - 1) {
      dir = (curvePoints[i + 1] - p).normalized();
    } else {
      dir = (p - curvePoints[i - 1]).normalized();
    }

    // Calculate the perpendicular vector for thickness
    Point perp = {-dir.y, dir.x};
    Point left = p + perp * (thickness / 2);
    Point right = p - perp * (thickness / 2);

    // Add vertices for the strip
    vertices.emplace_back(left.x);
    vertices.emplace_back(left.y);
    vertices.emplace_back(right.x);
    vertices.emplace_back(right.y);

    // Add indices for the strip
    if (i > 0) {
      int prevIndex = (i - 1) * 2;
      int currIndex = i * 2;

      // Two triangles per segment
      indices.emplace_back(prevIndex);    // Left side of previous segment
      indices.emplace_back(currIndex);    // Left side of current segment
      indices.emplace_back(prevIndex + 1); // Right side of previous segment

      indices.emplace_back(prevIndex + 1); // Right side of previous segment
      indices.emplace_back(currIndex);    // Left side of current segment
      indices.emplace_back(currIndex + 1); // Right side of current segment
    }

  }

  // Draw the triangles using the indexed triangle function
  draw_indexed_triangles(vertices.data(), vertices.size() / 2, indices.data(), indices.size());

  currTris = indices.size() / 3;
  currVerts = vertices.size() / 2;
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
      fillTris += 2;
      currVerts += 4; // Increment vertex count
      //debugf("After quad %d: Triangle count: %u, Vertex count: %u\n", i + 1, fillTris, currVerts);
  }
}

// Function to draw a filled shape between 2 Bézier curves
void Render::draw_filled_beziers(const Point& p0, const Point& p1, const Point& p2, const Point& p3, 
                               const Point& q0, const Point& q1, const Point& q2, const Point& q3, 
                               int segments) {
    std::vector<Point> upper_curve;
    std::vector<Point> lower_curve;

    currVerts = 0;
    fillTris = 0;
    //debugf("After reset: Triangle count: %u, Vertex count: %u\n", fillTris, currVerts);

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
        upper_curve.emplace_back(p);
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
        lower_curve.emplace_back(q);
    }

    // Fill the area between the two curves
    fill_between_beziers(lower_curve, upper_curve);
    //debugf("After fill_between_beziers: Triangle count: %u, Vertex count: %u\n", fillTris, currVerts);
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

    curvePoints.emplace_back(p);
  }

  // Close the polygon by connecting the last point back to the first
  curvePoints.emplace_back(curvePoints[0]);

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

void Render::draw_fan_transform(const std::vector<Point>& points, float angle, int segments, float rx, float ry) {

  // Copy original points
  std::vector<Point> transformedPoints = points;

  // Move only the outer points base on radii
  for (size_t i = 0; i < transformedPoints.size(); ++i) {
    transformedPoints[i].add(Point(rx, ry));
  }

  // Draw the ellipse using transformed points
  // Calculate the center and radii of the transformed points for drawing
  float cx = 0.0f, cy = 0.0f;
  float rx2 = 0.0f, ry2 = 0.0f;

  for (const auto& point : transformedPoints) {
    cx += point.x;
    cy += point.y;
  }

  cx /= transformedPoints.size();
  cy /= transformedPoints.size();

  for (const auto& point : transformedPoints) {
    float dx = point.x - cx;
    float dy = point.y - cy;
    rx2 = fmaxf(rx, fabsf(dx));
    ry2 = fmaxf(ry, fabsf(dy));
  }

  // Draw the ellipse with the calculated center and radii
  draw_ellipse(cx, cy, rx2, ry2, angle, (float)segments*0.01f);
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

