void draw_ellipse(float cx, float cy, float rx, float ry, int segments, std::vector<Point>& points)
Draws an ellipse and stores points around its perimeter.

Parameters:
cx (float): X-coordinate of the ellipse's center.
cy (float): Y-coordinate of the ellipse's center.
rx (float): Radius along the X-axis.
ry (float): Radius along the Y-axis.
segments (int): Number of segments to approximate the ellipse.
points (std::vector<Point>&): Vector to store the points around the ellipse's perimeter.

void draw_fan_curved(const std::vector<Point>& points)
Draws a curved triangle fan.

Parameters:
points (const std::vector<Point>&): Vector of points forming the fan. The first point is the center of the fan.

void draw_line(float x1, float y1, float x2, float y2, float thickness)
Draws a line segment of a certain thickness using two triangles.

Parameters:
x1 (float): X-coordinate of the starting point.
y1 (float): Y-coordinate of the starting point.
x2 (float): X-coordinate of the ending point.
y2 (float): Y-coordinate of the ending point.
thickness (float): Thickness of the line.

void draw_bezier_curve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments, float thickness)
Draws a Bézier curve using line segments.

Parameters:
p0 (const Point&): First control point.
p1 (const Point&): Second control point.
p2 (const Point&): Third control point.
p3 (const Point&): Fourth control point.
segments (int): Number of segments to approximate the curve.
thickness (float): Thickness of the line segments.

void fill_between_beziers(const std::vector<Point>& curve1, const std::vector<Point>& curve2)
Fills the area between two Bézier curves using triangles.

Parameters:
curve1 (const std::vector<Point>&): Points of the first Bézier curve.
curve2 (const std::vector<Point>&): Points of the second Bézier curve.

void draw_filled_beziers(const Point& p0, const Point& p1, const Point& p2, const Point& p3, const Point& q0, const Point& q1, const Point& q2, const Point& q3, int segments)
Draws a filled shape between two Bézier curves.

Parameters:
p0, p1, p2, p3 (const Point&): Control points of the first Bézier curve.
q0, q1, q2, q3 (const Point&): Control points of the second Bézier curve.
segments (int): Number of segments to approximate the curves.

bool is_ear(const std::vector<Point>& polygon, int u, int v, int w, const std::vector<int>& V)
Checks if a triangle is an "ear" in the context of polygon triangulation.

Parameters:

polygon (const std::vector<Point>&): Vector of points forming the polygon.
u, v, w (int): Indices of the vertices forming the triangle.
V (const std::vector<int>&): Vector of vertex indices.
Returns:

bool: true if the triangle is an ear, false otherwise.

void triangulate_polygon(const std::vector<Point>& polygon, std::vector<Point>& triangles)
Triangulates a polygon using the ear clipping algorithm.

Parameters:
polygon (const std::vector<Point>&): Vector of points forming the polygon.
triangles (std::vector<Point>&): Vector to store the resulting triangles.

void draw_filled_bezier_shape(const Point& p0, const Point& p1, const Point& p2, const Point& p3, int segments)
Draws a Bézier curve using line segments, then fills the shape with triangles.

Parameters:
p0, p1, p2, p3 (const Point&): Control points of the Bézier curve.
segments (int): Number of segments to approximate the curve.

void draw_fan_transform(const std::vector<Point>& point, const std::vector<float>& angle, int segments, float width)
Draws a transformed triangle fan based on width.

Parameters:
point (const std::vector<Point>&): Vector of points forming the fan.
angle (const std::vector<float>&): Vector of angles for transformation.
segments (int): Number of segments in the fan.
width (float): Width of the fan.

void fill_edge_to_ellipse(const std::vector<Point>& currentPoints, int segments, float scale)
Fills the area between the edges and an ellipse using triangles.

Parameters:
currentPoints (const std::vector<Point>&): Vector of points forming the current edge.
segments (int): Number of segments.
scale (float): Scaling factor for the points.