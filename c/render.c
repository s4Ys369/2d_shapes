#include <libdragon.h>
#include "rdpq/rdpq_fan.h"
#include "point.h"
#include "shapes.h"
#include "render.h"

void set_render_color(color_t color){
  rdpq_sync_pipe();
  rdpq_set_prim_color(color);
}

color_t get_random_render_color() {
  const color_t colors[] = {
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    INDIGO,
    VIOLET
  };
    
  int index = rand() % 7;
  return colors[index];
}

void set_random_render_color() {
  set_render_color(get_random_render_color());
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
void render_get_ellipse_points(PointArray* previousPoints, Point center, float rx, float ry, int segments, float fraction) {
  // Ensure previousPoints is properly initialized
  if (previousPoints == NULL) {
    debugf("point array is NULL\n");
    return;
  }

  // Initialize the PointArray if not already done
  if (previousPoints->points == NULL) {
    init_point_array(previousPoints);
  }

  if (segments == 0) {
    segments = 1;
  }

  // Clear any existing points in previousPoints
  if (previousPoints->points != NULL) {
    previousPoints->points = NULL;
  }
  previousPoints->count = 0;

  // Compute the angle range based on the fraction
  float maxAngle = 2.0f * M_PI * fraction; // Adjust the angle range

  // Compute points for the ellipse
  float angleStep = maxAngle / (float)segments;
  for (int i = 0; i < segments; ++i) {
    float angle = i * angleStep;
    float x = center.x + rx * fm_cosf(angle);
    float y = center.y + ry * fm_sinf(angle);
    add_point(previousPoints, x, y);

    // Check if point addition failed
    if (previousPoints->points == NULL) {
      debugf("Failed to add point to PointArray\n");
      return;
    }
  }
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
    if (i + 2 >= index_count) {
      debugf("Index array out of bounds\n");
      break;
    }
    int idx1 = indices[i];
    int idx2 = indices[i + 1];
    int idx3 = indices[i + 2];
        
    // Check if indices are within valid range
    if (idx1 < 0 || idx2 < 0 || idx3 < 0 || idx1 * 2 + 1 >= vertex_count || idx2 * 2 + 1 >= vertex_count || idx3 * 2 + 1 >= vertex_count) {
      debugf("Vertex index out of bounds: idx1=%d, idx2=%d, idx3=%d\n", idx1, idx2, idx3);
      continue;
    }

    // Retrieve vertex coordinates
    float v1[] = { vertices[idx1 * 2], vertices[idx1 * 2 + 1] };
    float v2[] = { vertices[idx2 * 2], vertices[idx2 * 2 + 1] };
    float v3[] = { vertices[idx3 * 2], vertices[idx3 * 2 + 1] };

    // Draw the triangle
    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    triCount++;
    vertCount++;
  }
}

void draw_rdp_fan(const PointArray* pa, const Point center, bool closed) {

  float cv[] = { center.x, center.y };
  float v1[] = { pa->points[0].x, pa->points[0].y };

  rdpq_fan_begin(&TRIFMT_FILL, cv);
  rdpq_fan_add_vertex(v1);
  vertCount++;

  for (size_t i = 0; i < pa->count; ++i) {
    float vertex[] = { pa->points[i].x, pa->points[i].y };
    rdpq_fan_add_vertex(vertex);
    triCount++;
    vertCount++;
  }

  if (closed){
    rdpq_fan_end();
  } else {
    rdpq_fan_destroy();
  }
  

}

// Function to draw a triangle fan from an array of points
void draw_fan(const PointArray* pa, const Point center) {
  if (pa->count < 2){ debugf("Need at least 3 points to form a triangle"); return; }

  for (size_t i = 0; i < pa->count - 1; ++i) {
    Point p2 = pa->points[i];
    Point p3 = pa->points[i + 1];

    float v1[] = { center.x, center.y };
    float v2[] = { p2.x, p2.y };
    float v3[] = { p3.x, p3.y };

    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    triCount++;
    vertCount += 2;
  }

  // Add a final triangle to close the loop
  Point lastPoint = pa->points[pa->count - 1];
  Point firstPoint = pa->points[0];

  float lastV1[] = { center.x, center.y };
  float lastV2[] = { lastPoint.x, lastPoint.y };
  float lastV3[] = { firstPoint.x, firstPoint.y };

  rdpq_triangle(&TRIFMT_FILL, lastV1, lastV2, lastV3);
  triCount++;

}

// Function to draw a triangle fan from an array of points
void draw_strip(float* v1, float* v2, float* v3, float* v4) {

  rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
  rdpq_triangle(&TRIFMT_FILL, v2, v4, v3);
  rdpq_sync_pipe();
  triCount += 2;
  vertCount += 4;


}

// Function to draw a strip of triangles from an array of vertices
void draw_strip_from_array(float* vertices, int vertexCount, float width) {
  if (vertexCount < 2) {
    debugf("Not enough vertices to draw a strip\n");
    return;
  }

  // Calculate the number of quads and the total number of vertices needed
  int quadCount = vertexCount - 1;
  int totalVertices = quadCount * 8; // 8 floats per quad (4 vertices, 2 coords each)
  float* stripVertices = (float*)malloc(totalVertices * sizeof(float));

  if (!stripVertices) {
    debugf("Strip vertices allocation failed\n");
    return;
  }

  // Calculate perpendicular vectors for the width
  for (int i = 0; i < vertexCount - 1; ++i) {
    float dx = vertices[(i + 1) * 2] - vertices[i * 2];
    float dy = vertices[(i + 1) * 2 + 1] - vertices[i * 2 + 1];
    float length = sqrtf(dx * dx + dy * dy);
    if (length == 0) {
      continue; // Avoid division by zero for overlapping points
    }
    float ux = dy / length; // Unit perpendicular vector x
    float uy = -dx / length; // Unit perpendicular vector y

    // Calculate the offset for width
    float offsetX = ux * width * 0.5f;
    float offsetY = uy * width * 0.5f;

    // Set vertices for the quad
    int index = i * 8;
    stripVertices[index] = vertices[i * 2] + offsetX;
    stripVertices[index + 1] = vertices[i * 2 + 1] + offsetY;
    stripVertices[index + 2] = vertices[i * 2] - offsetX;
    stripVertices[index + 3] = vertices[i * 2 + 1] - offsetY;
    stripVertices[index + 4] = vertices[(i + 1) * 2] + offsetX;
    stripVertices[index + 5] = vertices[(i + 1) * 2 + 1] + offsetY;
    stripVertices[index + 6] = vertices[(i + 1) * 2] - offsetX;
    stripVertices[index + 7] = vertices[(i + 1) * 2 + 1] - offsetY;
  }

  // Draw the quads
  for (int i = 0; i < quadCount; ++i) {
    float v1[] = { stripVertices[i * 8], stripVertices[i * 8 + 1] };
    float v2[] = { stripVertices[i * 8 + 2], stripVertices[i * 8 + 3] };
    float v3[] = { stripVertices[i * 8 + 4], stripVertices[i * 8 + 5] };
    float v4[] = { stripVertices[i * 8 + 6], stripVertices[i * 8 + 7] };

    // Draw the two triangles for each quad
    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    rdpq_triangle(&TRIFMT_FILL, v2, v4, v3);
    triCount += 2;
    vertCount += 4;
  }

  // Free the allocated memory
  free(stripVertices);
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

  if (area <= 0.9f) {
    // If only drawing subpixels, exit
    debugf("Do you really need subpixels?\n");
    return;
  } else if (area >= 1.0f && area < 2.9f) {
    // If only drawing ~4 pixels or less, just draw a quad to save triangles
    draw_quad(cx - offset, cy - offset, cx + offset, cy + offset, angle, 1.0f);
    return;
  } else {
    // Default segments calculation for areas > 2.9f
    segments = (int)((area < 3.0f) ? 3.0f : area) / ((area >= 9.9f) ? 2 : 3);

    // Enforce a minimum of 6 segments
    segments = segments < 6 ? 6 : segments;

    // Enforce a maximum of 200 segments for high detail levels
    if (area > 9.9f) {
      segments = (segments > 200 || lod > 2.0f) ? 200 : segments;
    }

    // debugf("Segments %u\n", segments);
  }

  // Calculate angles for position
  float theta = 2.0f * M_PI / (float)segments;
  float cos_theta = fm_cosf(theta);
  float sin_theta = fm_sinf(theta);

  // Calculate angles for rotation
  float cos_angle = fm_cosf(angle);
  float sin_angle = fm_sinf(angle);

  // Initialize PointArray
  PointArray pa = { .count = segments, .points = malloc(segments * sizeof(Point)) };
  if (!pa.points) {
    debugf("Point array allocation failed\n");
    return;
  }

  // Calculate perimeter vertices
  float x = rx;
  float y = 0.0f;

  // Per segment/triangle
  for (int i = 0; i < segments; ++i) {

    // Apply rotation
    float rotatedX = x * cos_angle - y * sin_angle;
    float rotatedY = x * sin_angle + y * cos_angle;
    pa.points[i].x = cx + rotatedX;
    pa.points[i].y = cy + rotatedY;

    // Calculate next position from rotation matrix
    float nextX = cos_theta * x - sin_theta * y;
    float nextY = sin_theta * x + cos_theta * y;

    // Set position for next iteration
    x = nextX;
    y = nextY;
  
  }

  //debugf("Total vertices: %d\n", vertex_count);
  draw_rdp_fan(&pa, pa.points[0], true);

  free(pa.points);


}

// Function to draw a quad/rectangle of certain thickness with rotation and scale, using a 2 triangle strip
void draw_line(float x1, float y1, float x2, float y2, float thickness) {

  // Check for subpixel thickness
  if(thickness <= 0.9f){
    thickness = 1.0f;
  }

  // Define points
  Point start = point_new(x1, y1);
  Point end = point_new(x2, y2);

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

  // Compute the points for the line
  Point p1_left = { start.x + perp.x, start.y + perp.y };
  Point p1_right = { start.x - perp.x, start.y - perp.y };
  Point p2_left = { end.x + perp.x, end.y + perp.y };
  Point p2_right = { end.x - perp.x, end.y - perp.y };

  // Define vertices for two triangles to form the line with thickness
  float v1[] = { p1_left.x, p1_left.y };
  float v2[] = { p1_right.x, p1_right.y };
  float v3[] = { p2_left.x, p2_left.y };
  float v4[] = { p2_right.x, p2_right.y };

  // Draw two triangles to form the line
  draw_strip(v1,v2,v3,v4);
}

// Function to draw a quad/rectangle of certain thickness with rotation and scale, using a 2 triangle strip
void draw_quad(float x1, float y1, float x2, float y2, float angle, float thickness) {

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
  draw_strip(v1,v2,v3,v4);
}

// Function to draw a quad/rectangle outline with rotation and thickness
void draw_quad_outline(float x1, float y1, float x2, float y2, float angle, float thickness) {

  // Check for subpixel thickness
  if (thickness <= 0.9f) {
    thickness = 1.0f;
  }

  // Define points for the corners of the quad
  Point start = point_new(x1, y1);
  Point end = point_new(x2, y2);

  // Calculate the perpendicular vector for the thickness
  Point perp = point_new((thickness/2.0f), (thickness/2.0f));

  // Compute the points for the four corners of the quad (p1 to p4)
  Point p1 = { start.x , start.y - perp.y };
  Point p2 = { end.x , start.y - perp.y };
  Point p3 = { start.x , end.y};
  Point p4 = end;

  // Draw the outline by connecting the four points
  draw_quad(p1.x+(thickness/2), p1.y+(thickness/2), p3.x, p3.y-(thickness/2), angle, 1);  // Left side outline
  draw_quad(p2.x-(thickness/2), p2.y+(thickness/2), p4.x, p4.y-(thickness/2), angle, 1);  // Right side outline
  draw_quad(p1.x, p1.y+(thickness), p2.x, p2.y+(thickness), angle, thickness);  // Top cap outline (start)
  draw_quad(p3.x, p3.y, p4.x, p4.y, angle, thickness);  // Bottom cap outline (end)

}


// Function to draw a Bézier curve as a triangle strip with a given thickness
void draw_bezier_curve(const Point* p0, const Point* p1, const Point* p2, const Point* p3, int segments, float angle, float thickness) {

  // Initialize array
  PointArray* curvePoints = (PointArray*)malloc_uncached(sizeof(PointArray)); 
  if (!curvePoints) {
    debugf("Failed to allocate memory for curvePoints\n");
    return;
  }
  init_point_array(curvePoints);
  if (!curvePoints->points) {
    debugf("Failed to initialize curvePoints->points\n");
    return;
  }

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

    add_point(curvePoints, x, y);
  }

  // Center of the curve for rotation ??? FIXME
  //Point center = point_new((p0->x + p3->x) / 2.0f, (p0->y + p3->y) / 2.0f);

  float cos_angle = fm_cosf(angle);
  float sin_angle = fm_sinf(angle);

  for (int i = 0; i < curvePoints->count; ++i) { // FIXME: Use point_normalized and rotate_line_points
    Point p = curvePoints->points[i];
        
    // Compute the normal vector for the curve point
    float nx = 0, ny = 0;
    if (i < curvePoints->count - 1) {
      float dx = curvePoints->points[i + 1].x - p.x;
      float dy = curvePoints->points[i + 1].y - p.y;
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
    if (i < curvePoints->count - 1) {
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
  draw_indexed_triangles(vertices, vertexCount, indices, indexCount);

  free(vertices);
  free(indices);

  currTris = indexCount / 3;
  currVerts = vertexCount / 2;
  free(curvePoints->points);
  free_uncached(curvePoints);
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
  PointArray* topCurvePoints = (PointArray*)malloc_uncached(sizeof(PointArray)); 
  PointArray* bottomCurvePoints = (PointArray*)malloc_uncached(sizeof(PointArray)); 
  init_point_array(topCurvePoints);
  init_point_array(bottomCurvePoints);

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

    add_point(topCurvePoints, x, y);
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

    add_point(bottomCurvePoints, x, y);
  }

    // Fill the area between the two curves
    fill_between_beziers(topCurvePoints, bottomCurvePoints);
    //debugf("After fill_between_beziers: Triangle count: %u, Vertex count: %u\n", fillTris, currVerts);
    free(topCurvePoints->points);
    free(bottomCurvePoints->points);
    free_uncached(topCurvePoints);
    free_uncached(bottomCurvePoints);
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

  PointArray* curvePoints = (PointArray*)malloc_uncached(sizeof(PointArray)); 
  init_point_array(curvePoints);

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

    add_point(curvePoints, x, y);
  }

  // Close the polygon by connecting the last point back to the first
  add_existing_point(curvePoints, curvePoints->points[0]);

  // Triangulate the closed polygon (using a simple ear clipping method)
  PointArray* triangles = (PointArray*)malloc_uncached(sizeof(PointArray));
  init_point_array(triangles);
  triangulate_polygon(curvePoints, triangles);

  // Draw the triangles
  for (size_t i = 0; i < triangles->count; i += 3) {
    float v1[] = { triangles->points[i].x, triangles->points[i].y };
    float v2[] = { triangles->points[i + 1].x, triangles->points[i + 1].y };
    float v3[] = { triangles->points[i + 2].x, triangles->points[i + 2].y };

    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    triCount++;
    vertCount += 2;
  }

  free(curvePoints->points);
  free(triangles->points);
  free_uncached(curvePoints);
  free_uncached(triangles);
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
}

// Function to draw a quad/rectangle from the edge of an ellipse/fan to the edge of a "line" (ie another quad/rectangle)
void fill_edge_ellipse_to_line(PointArray* previousPoints, PointArray* currentPoints, int segments, float scale) {
  Point prevCenter = point_default();
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

