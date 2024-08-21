# Rendering Utility Functions

This file provides various functions for rendering shapes and performing operations on points. The functions include basic transformations, drawing routines, and utilities for handling Bézier curves and polygons.

## Functions
### void set_render_color(color_t color);
Sets the rendering color.

**Parameters:**

- `color` - The color to set for rendering.

### void render_move_point(PointArray* pa, size_t index, float dx, float dy);
Moves a point in the PointArray by a given offset.

**Parameters:**

- `pa` - Pointer to the PointArray.
- `index` - Index of the point to move.
- `dx` - Change in the x-coordinate.
- `dy` - Change in the y-coordinate.

### void render_move_shape_points(PointArray* pa, float dx, float dy);
Moves all points in the PointArray by a given offset.

**Parameters:**

- `pa` - Pointer to the PointArray.
- `dx` - Change in the x-coordinate.
- `dy` - Change in the y-coordinate.

### void render_rotate_point(PointArray* pa, size_t index, Point center, float angle);
Rotates a point around a given center.

**Parameters:**

- `pa` - Pointer to the PointArray.
- `index` - Index of the point to rotate.
- `center` - The center of rotation.
- `angle` - The angle of rotation in radians.

### void render_rotate_shape_points(PointArray* pa, Point center, float angle);
Rotates all points in the PointArray around a given center.

**Parameters:**

- `pa` - Pointer to the PointArray.
- `center` - The center of rotation.
- `angle` - The angle of rotation in radians.

### void render_get_ellipse_points(PointArray* previousPoints, Point center, float rx, float ry, int segments);
Computes points around an ellipse and stores them in the PointArray.

**Parameters:**

- `previousPoints` - Pointer to the PointArray to store ellipse points.
- `center` - The center of the ellipse.
- `rx` - Radius in the x direction.
- `ry` - Radius in the y direction.
- `segments` - Number of segments to approximate the ellipse.


### void draw_indexed_triangles(float* vertices, int vertex_count, int* indices, int index_count);
Draws triangles from vertex and index arrays.

**Parameters:**

- `vertices` - Array of vertex coordinates.
- `vertex_count` - Number of vertices.
- `indices` - Array of vertex indices.
- `index_count` - Number of indices.


### void draw_fan(const PointArray* pa, const Point center);
Draws a triangle fan from an array of points.

**Parameters:**

- `pa` - Pointer to the PointArray containing the fan points.
- `center` - Center of the triangle fan as a Point.

### void draw_circle(float cx, float cy, float rx, float ry, float angle, float lod);
Draws a circle as a triangle fan.

**Parameters:**

- `cx` - x-coordinate of the circle center.
- `cy` - y-coordinate of the circle center.
- `rx` - Radius in the x direction.
- `ry` - Radius in the y direction.
- `angle` - Rotation angle in radians.
- `lod` - Level of detail.

### void draw_line(float x1, float y1, float x2, float y2, float angle, float thickness);
Draws a line with a specified thickness and rotation.

**Parameters:**

- `x1, y1` - Starting point coordinates.
- `x2, y2` - Ending point coordinates.
- `angle` - Rotation angle in radians.
- `thickness` - Thickness of the line.

### void draw_bezier_curve(const Point* p0, const Point* p1, const Point* p2, const Point* p3, int segments, float angle, float thickness);
Draws a Bézier curve as a triangle strip.

**Parameters:**

- `p0, p1, p2, p3` - Control points of the Bézier curve.
- `segments` - Number of segments to approximate the curve.
- `angle` - Rotation angle in radians.
- `thickness` - Thickness of the curve.


### void fill_between_beziers(const PointArray* curve1, const PointArray* curve2);
Fills the area between two Bézier curves using quads (rectangles).

**Parameters:**

- `curve1` - Points defining the first Bézier curve.
- `curve2` - Points defining the second Bézier curve.

### void draw_filled_beziers(const Point* p0, const Point* p1, const Point* p2, const Point* p3, const Point* q0, const Point* q1, const Point* q2, const Point* q3, int segments);
Draws a filled shape between two Bézier curves.

**Parameters:**

- `p0, p1, p2, p3` - Control points for the first Bézier curve.
- `q0, q1, q2, q3` - Control points for the second Bézier curve.
- `segments` - Number of segments to use for approximating the Bézier curves.

### bool is_ear(const PointArray* polygon, int u, int v, int w, const int* V);
Checks if a triangle formed by three consecutive vertices in a polygon is an "ear" (a triangle that does not contain any other vertices of the polygon inside it).

**Parameters:**

- `polygon` - Points defining the polygon.
- `u, v, w` - Indices of the vertices forming the triangle.
- `V` - Array of vertex indices.
- returns true if the triangle is an ear, false otherwise.

### void triangulate_polygon(const PointArray* polygon, PointArray* triangles);
Triangulates a polygon using a simple ear clipping algorithm.

**Parameters:**

- `polygon` - Points defining the polygon to be triangulated.
- `triangles` - Array to store the resulting triangles.

### void draw_filled_bezier_shape(const Point* p0, const Point* p1, const Point* p2, const Point* p3, int segments);
Draws a Bézier curve and fills the shape with triangles.

**Parameters:**

- `p0, p1, p2, p3` - Control points for the Bézier curve.
- `segments` - Number of segments to use for approximating the Bézier curve.

### void draw_fan_transform(const PointArray* fan, float angle, int segments, float rx, float ry);
Draws a fully transformable triangle fan.

**Parameters:**

- `fan` - Points defining the triangle fan.
- `angle` - Rotation angle in radians.
- `segments` - Number of segments for the fan.
- `rx, ry` - Radii for transforming the fan points.

### void fill_edge_ellipse_to_line(PointArray* previousPoints, PointArray* currentPoints, int segments, float scale);
Draws a quad/rectangle from the edge of an ellipse (or fan) to the edge of a line (another quad/rectangle).

**Parameters:**

- `previousPoints` - Points defining the previous ellipse or fan.
- `currentPoints` - Points defining the current ellipse or fan.
- `segments` - Number of segments in the ellipse or fan.
- `scale` - Scaling factor for the points.
- `Note:` previousPoints is updated to currentPoints after drawing.
