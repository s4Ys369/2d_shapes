Point Class Documentation
Constructors
Point::Point(float x, float y)

Description: Constructor that initializes a Point object with given x and y coordinates.

Parameters:

x: The x-coordinate of the point.
y: The y-coordinate of the point.
Methods
void Point::add(const Point& v)

Description: Adds the coordinates of another Point to the current Point.

Parameters:

v: The Point to add to the current Point.
Point Point::add(const Point& v1, const Point& v2)

Description: Returns a new Point that is the result of adding two Point objects.

Parameters:

v1: The first Point.
v2: The second Point.
Returns: A new Point which is the sum of v1 and v2.

Point Point::sub(const Point& v1, const Point& v2)

Description: Returns a new Point that is the result of subtracting one Point from another.

Parameters:

v1: The point from which v2 is subtracted.
v2: The point to subtract from v1.
Returns: A new Point which is the difference between v1 and v2.

float Point::heading() const

Description: Computes the angle (heading) of the vector from the origin to the current Point, in radians.

Returns: The angle in radians between the positive x-axis and the vector.

Point Point::from_angle(float angle)

Description: Creates a Point representing a vector with unit length in the direction of the given angle.

Parameters:

angle: The angle in radians.
Returns: A new Point whose coordinates correspond to a unit vector in the direction of angle.

float Point::magnitude() const

Description: Calculates the magnitude (length) of the vector represented by the current Point.

Returns: The magnitude of the vector.

void Point::normalize()

Description: Normalizes the vector represented by the current Point, making its length equal to 1.

Point& Point::set_mag(float newMag)

Description: Sets the magnitude of the vector represented by the current Point to a new value.

Parameters:

newMag: The new magnitude of the vector.
Returns: The updated Point object with the new magnitude.

Point Point::copy() const

Description: Creates a copy of the current Point.

Returns: A new Point with the same coordinates as the current Point.

Point Point::scale(const Point& center, const Point& point, float scale)

Description: Scales a point outward from a center by the given scale factor.

Parameters:

center: The point from which scaling occurs.
point: The point to be scaled.
scale: The scale factor.
Returns: A new Point that is the result of scaling point from center.

Point Point::translate(Point p, float dx, float dy)

Description: Translates a Point by adding dx and dy to its coordinates.

Parameters:

p: The point to be translated.
dx: The amount to translate in the x-direction.
dy: The amount to translate in the y-direction.
Returns: A new Point that is the result of translating p.

Point Point::rotate(Point p, float angle)

Description: Rotates a Point around the origin by a given angle.

Parameters:

p: The point to be rotated.
angle: The rotation angle in radians.
Returns: A new Point that is the result of rotating p by angle.

Point Point::transform(const Point& point, float angle, float width)

Description: Transforms a Point by translating it along a direction given by an angle and distance.

Parameters:

point: The point to be transformed.
angle: The direction of the transformation in radians.
width: The distance to translate.
Returns: A new Point after applying the transformation.

float Point::epsilon_test(const Point& A, const Point& B, const Point& C)

Description: Performs an epsilon test to determine the orientation of the point C relative to the line segment from A to B.

Parameters:

A: The start point of the line segment.
B: The end point of the line segment.
C: The point to test.
Returns: A value indicating the relative orientation; positive if C is to the left of the line segment, negative if to the right, and zero if collinear.

bool Point::point_in_triangle(const Point& P, const Point& A, const Point& B, const Point& C)

Description: Checks if a point P is inside the triangle defined by points A, B, and C.

Parameters:

P: The point to check.
A, B, C: The vertices of the triangle.
Returns: true if P is inside or on the boundary of the triangle, false otherwise.