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
    Point p = point_new(dx,dy);
    point_add_in_place(&pa->points[index], &p);
  }
}
 
void render_move_shape_points(PointArray* pa, float dx, float dy) {
  Point direction = point_new(dx,dy);
  for (int i = 0; i < pa->count; ++i) {
    point_add_in_place(&pa->points[i], &direction);
  }
}

void render_rotate_point(PointArray* pa, size_t index, Point center, float angle) {
  if (index < pa->count) {
    point_rotate(&pa->points[index], &center, angle);
  }
}

void render_rotate_shape_points(PointArray* pa, Point center, float angle) {
  for (int i = 0; i < pa->count; ++i) {
    point_rotate(&pa->points[i], &center, angle);
  }
}

// Function to get points around an ellipse
PointArray render_get_ellipse_points(Point center, float rx, float ry, int segments) {

  if(segments == 0){
    segments = 1;
  }

  PointArray pa; init_point_array(&pa);
  pa.points = (Point*)malloc(sizeof(Point) * segments);
  pa.count = segments;

  float angleStep = 2 * M_PI / segments;
  for (int i = 0; i < segments; ++i) {
    float angle = i * angleStep;
    float x = center.x + rx * fm_cosf(angle);
    float y = center.y + ry * fm_sinf(angle);
    pa.points[i] = (Point){x, y};
  }

  return pa;
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
void draw_fan(const PointArray* pa) {
  if (pa->count < 3){ debugf("Need at least 3 points to form a triangle"); return; }

  // First point is the center of the fan
  Point center = pa->points[0];

  for (size_t i = 1; i < pa->count - 1; ++i) {
    Point v1 = center;
    Point v2 = pa->points[i];
    Point v3 = pa->points[i + 1];

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
  float theta = 2.0f * M_PI / (float)segments;
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
  int* indices = NULL;
  int *index_count = segments * 3; // Each triangle uses 3 indices
  indices = create_triangle_fan_indices(segments, &index_count);

  // Draw the indexed vertices
  draw_indexed_triangles(&vertices[0], vertex_count, &indices[0], index_count);

}

// Function to draw a quad/rectangle of certain thickness with rotation and scale, using a 2 triangle strip
void draw_line(float x1, float y1, float x2, float y2, float angle, float thickness) {

  // Check for subpixel thickness
  if(thickness <= 0.9f){
    thickness = 1.0f;
  }

  // Define points
  Point start = point_new(x1, y1);
  Point end = point_new(x2, y2);

  // Calculate the center of the line segment
  Point center = point_new((x1 + x2) / 2.0f, (y1 + y2) / 2.0f);

  // Calculate direction vector
  Point direction = point_sub(&end, &start);
  float length =  point_magnitude(&direction);

  // Check for non-zero length and normalize
  if (length != 0) {
    point_normalize(&direction);
  } else {
    debugf("Line length cannot be 0");  
    return;
  }

  // Calculate the perpendicular vector for the thickness
  Point perp = point_new(-direction.y, direction.x); // Perpendicular to direction
  perp = point_set_mag(&perp, thickness / 2); // Set the magnitude to half of the thickness

  // Rotation matrix
  float cos_angle = fm_cosf(angle);
  float sin_angle = fm_sinf(angle);

  // Compute the points for the line
  Point p1_left = { start.x , start.y - perp.y };
  Point p1_right = { end.x , start.y - perp.y };
  Point p2_left = { start.x , end.y};
  Point p2_right = end;

  // Rotate the trapezoid vertices
  rotate_line_point(&p1_left,  &center, cos_angle, sin_angle);
  rotate_line_point(&p1_right, &center, cos_angle, sin_angle);
  rotate_line_point(&p2_left,  &center, cos_angle, sin_angle);
  rotate_line_point(&p2_right, &center, cos_angle, sin_angle);

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
void draw_bezier_curve(const Point* p0, const Point* p1, const Point* p2, const Point* p3, int segments, float angle, float thickness) {

  // Initialize arrays
  PointArray curvePoints; init_point_array(&curvePoints);

  float* vertices = NULL;
  int vertexCount = 0;

  int* indices = NULL;
  int indexCount = 0;

  float step = (segments != 0) ? 1.0f / (float)segments : 1.0f;

  // Compute Bézier curve points FIXME: precompute?
  for (int i = 0; i <= segments; ++i) {
    float t = i * step;
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    float x = uuu * p0->x + 3 * uu * t * p1->x + 3 * u * tt * p2->x + ttt * p3->x;
    float y = uuu * p0->y + 3 * uu * t * p1->y + 3 * u * tt * p2->y + ttt * p3->y;

    add_point(&curvePoints, x, y);
  }

  // Center of the curve for rotation
  Point center = point_new((p0->x + p3->x) / 2.0f, (p0->y + p3->y) / 2.0f);

  float cos_angle = fm_cosf(angle);
  float sin_angle = fm_sinf(angle);

  for (int i = 0; i < curvePoints.count; ++i) { // FIXME: Use point_normalized and rotate_line_points
    Point p = curvePoints.points[i];
        
    // Compute the normal vector for the curve point
    float nx = 0, ny = 0;
    if (i < curvePoints.count - 1) {
      float dx = curvePoints.points[i + 1].x - p.x;
      float dy = curvePoints.points[i + 1].y - p.y;
      float length = sqrtf(dx * dx + dy * dy);
      if(length != 0){
        nx = -dy / length * thickness / 2;
        ny = dx / length * thickness / 2;
      } else {
        nx = -dy * thickness / 2;
        ny = dx * thickness / 2;
      }
    }

    // Apply rotation
    float offsetX = nx * cos_angle - ny * sin_angle;
    float offsetY = nx * sin_angle + ny * cos_angle;

    // Add vertices for the top and bottom of the strip
    add_vertex(&vertices, &vertexCount, p.x + offsetX, p.y + offsetY);
    add_vertex(&vertices, &vertexCount, p.x - offsetX, p.y - offsetY);

    // Add indices
    if (i < curvePoints.count - 1) {
      int baseIndex = i * 2;
      add_index(&indices, &indexCount, baseIndex);
      add_index(&indices, &indexCount, baseIndex + 1);
      add_index(&indices, &indexCount, baseIndex + 2);
      add_index(&indices, &indexCount, baseIndex + 1);
      add_index(&indices, &indexCount, baseIndex + 3);
      add_index(&indices, &indexCount, baseIndex + 2);
    }
  }

  // Draw the triangles using the indexed triangle function
  draw_indexed_triangles(&vertices, vertexCount / 2, &indices, indexCount);

  currTris = indexCount / 3;
  currVerts = vertexCount / 2;
  free(curvePoints.points);
  free(vertices);
  free(indices);
}


// Function to fill area between 2 Bézier curves using quads/rectangles
void fill_between_beziers(const PointArray* curve1, const PointArray* curve2) {
  size_t size = fminf(curve1->count, curve2->count);
  for (size_t i = 0; i < size - 1; ++i) {
    float v1[] = { curve1->points[i].x, curve1->points[i].y };
    float v2[] = { curve1->points[i + 1].x, curve1->points[i + 1].y };
    float v3[] = { curve2->points[i].x, curve2->points[i].y };
    float v4[] = { curve2->points[i + 1].x, curve2->points[i + 1].y };

    // Draw two triangles to fill the quad
    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    rdpq_triangle(&TRIFMT_FILL, v2, v3, v4);
    fillTris += 2;
    currVerts += 4; // Increment vertex count
    //debugf("After quad %d: Triangle count: %u, Vertex count: %u\n", i + 1, fillTris, currVerts);
  }
}

// Function to draw a filled shape between 2 Bézier curves
void draw_filled_beziers(const Point* p0, const Point* p1, const Point* p2, const Point* p3, 
                               const Point* q0, const Point* q1, const Point* q2, const Point* q3, 
                               int segments) {


  // Set up two arrays
  PointArray topCurvePoints, bottomCurvePoints;
  init_point_array(&topCurvePoints);
  init_point_array(&bottomCurvePoints);

  // Reset accumulators
  currVerts = 0;
  fillTris = 0;
  //debugf("After reset: Triangle count: %u, Vertex count: %u\n", fillTris, currVerts);

  // Compute Bézier curve points FIXME: precompute?
  float step = (segments != 0) ? 1.0f / (float)segments : 1.0f;

  // Top curve
  for (int i = 0; i <= segments; ++i) {
    float t = i * step;
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    float x = uuu * p0->x + 3 * uu * t * p1->x + 3 * u * tt * p2->x + ttt * p3->x;
    float y = uuu * p0->y + 3 * uu * t * p1->y + 3 * u * tt * p2->y + ttt * p3->y;

    add_point(&topCurvePoints, x, y);
  }

  // Bottom curve
  for (int i = 0; i <= segments; ++i) {
    float t = i * step;
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    float x = uuu * q0->x + 3 * uu * t * q1->x + 3 * u * tt * q2->x + ttt * q3->x;
    float y = uuu * q0->y + 3 * uu * t * q1->y + 3 * u * tt * q2->y + ttt * q3->y;

    add_point(&bottomCurvePoints, x, y);
  }

    // Fill the area between the two curves
    fill_between_beziers(&topCurvePoints, &bottomCurvePoints);
    //debugf("After fill_between_beziers: Triangle count: %u, Vertex count: %u\n", fillTris, currVerts);
    free(&topCurvePoints);
    free(&bottomCurvePoints);
}

// Function to check ear clipping, An "ear" is a triangle formed by three consecutive vertices in a polygon that does not contain any other vertices of the polygon inside it.
bool is_ear(const PointArray* polygon, int u, int v, int w, const int* V) {
  Point A = polygon->points[V[u]];
  Point B = polygon->points[V[v]];
  Point C = polygon->points[V[w]];

  if (point_epsilon_test(&A, &B, &C) >= 0) {
    return false;
  }

  for (size_t p = 0; p < polygon->count; ++p) {
    if ((p == (size_t)u) || (p == (size_t)v) || (p == (size_t)w)) {
      continue;
    }
    if (point_in_triangle(&polygon->points[p], &A, &B, &C)) {
      return false;
    }
  }

  return true;
}

// A simple ear clipping algorithm for triangulation
void triangulate_polygon(const PointArray* polygon, PointArray* triangles) {

  int* V = (int*)malloc(polygon->count * sizeof(int));
  if (V == NULL) {
    debugf("Polygon point count cannot be 0\n");
    return;
  }
  for (size_t i = 0; i < polygon->count; ++i) {
    V[i] = i;
  }

  int n = polygon->count;
  int count = 2 * n;

  for (int v = n - 1; n > 2;) {
    if ((count--) <= 0) {
      debugf("No polygon detected\n");
      free(V);
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
    add_existing_point(triangles, polygon->points[a]);
    add_existing_point(triangles, polygon->points[b]);
    add_existing_point(triangles, polygon->points[c]);

    for (int s = v, t = v + 1; t < n; s++, t++) {
      V[s] = V[t];
    }
    n--;

    count = 2 * n;
  }

  free(V);
}

// Function to draw a Bézier curve using line segments, then fill shape with triangles. Note the base will always be a straight line.
void draw_filled_bezier_shape(const Point* p0, const Point* p1, const Point* p2, const Point* p3, int segments) {

  PointArray curvePoints; init_point_array(&curvePoints);

  float step = (segments != 0) ? 1.0f / (float)segments : 1.0f;

  // Compute Bézier curve points FIXME: precompute?
  for (int i = 0; i <= segments; ++i) {
    float t = i * step;
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    float x = uuu * p0->x + 3 * uu * t * p1->x + 3 * u * tt * p2->x + ttt * p3->x;
    float y = uuu * p0->y + 3 * uu * t * p1->y + 3 * u * tt * p2->y + ttt * p3->y;

    add_point(&curvePoints, x, y);
  }

  // Close the polygon by connecting the last point back to the first
  add_existing_point(&curvePoints, curvePoints.points[0]);

  // Triangulate the closed polygon (using a simple ear clipping method)
  PointArray triangles; init_point_array(&triangles);
  triangulate_polygon(&curvePoints, &triangles);

  // Draw the triangles
  for (size_t i = 0; i < triangles.count; i += 3) {
    float v1[] = { triangles.points[i].x, triangles.points[i].y };
    float v2[] = { triangles.points[i + 1].x, triangles.points[i + 1].y };
    float v3[] = { triangles.points[i + 2].x, triangles.points[i + 2].y };

    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    triCount++;
    vertCount += 2;
  }

  free(curvePoints.points);
  free(triangles.points);
}

// Function to draw a fully transformable triangle fan
void draw_fan_transform(const PointArray* fan, float angle, int segments, float rx, float ry) {

  // Create a transformed copy of the original points
  PointArray transformedFan;
  init_point_array(&transformedFan);

  // Move only the outer points based on radii
  for (size_t i = 0; i < fan->count; ++i) {
    Point transformPoint = point_new(rx, ry);
    Point newPoint = point_sum(&fan->points[i], &transformPoint);
    add_point(&transformedFan, newPoint.x, newPoint.y);
  }

  // Calculate the center and radii of the transformed points for drawing
  float cx = 0.0f, cy = 0.0f;
  float rx2 = 0.0f, ry2 = 0.0f;

  for (size_t i = 0; i < transformedFan.count; ++i) {
    cx += transformedFan.points[i].x;
    cy += transformedFan.points[i].y;
  }

  cx /= (float)transformedFan.count;
  cy /= (float)transformedFan.count;

  for (size_t i = 0; i < transformedFan.count; ++i) {
    float dx = transformedFan.points[i].x - cx;
    float dy = transformedFan.points[i].y - cy;
    rx2 = fmaxf(rx2, fabsf(dx));
    ry2 = fmaxf(ry2, fabsf(dy));
  }

  // Draw the ellipse with the calculated center and radii
  draw_circle(cx, cy, rx2, ry2, angle, (float)segments * 0.01f);

  free(transformedFan.points);
}

// Function to draw a quad/rectangle from the edge of an ellipse/fan to the edge of a "line" (ie another quad/rectangle)
void fill_edge_ellipse_to_line(const PointArray* currentPoints, int segments, float scale) {
  static PointArray* previousPoints;  // Static to persist between function calls
  init_point_array(previousPoints);
  Point prevCenter = point_default();
  add_existing_point(previousPoints, prevCenter);
  Point currCenter = point_default();

  if (previousPoints->count != 0) {

    // Calculate centers for previous and current points
    calculate_array_center(previousPoints, &prevCenter);
    calculate_array_center(currentPoints, &currCenter);

    // Scale points outward to fill in any gaps
    for (int i = 0; i < segments; ++i) {
      Point v1r = point_scale(&prevCenter, &previousPoints->points[i], scale);
      Point v2r = point_scale(&prevCenter, &previousPoints->points[(i + 1) % segments], scale); // Use modulo to wrap around
      Point v3r = point_scale(&currCenter, &currentPoints->points[i], scale);
      Point v4r = point_scale(&currCenter, &currentPoints->points[(i + 1) % segments], scale); // Use modulo to wrap around

      // Create triangles between scaled points
      float v1f[] = { v1r.x, v1r.y };
      float v2f[] = { v2r.x, v2r.y };
      float v3f[] = { v3r.x, v3r.y };
      float v4f[] = { v4r.x, v4r.y };

      // Draw two triangles to form a quad between the points
      rdpq_triangle(&TRIFMT_FILL, v1f, v2f, v3f);
      rdpq_triangle(&TRIFMT_FILL, v2f, v4f, v3f);
      triCount++;
      vertCount += 4;
    }
  }

  previousPoints = currentPoints; // Save current points for the next iteration
}
