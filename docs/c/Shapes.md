# Shapes Module Documentation

## Overview
This module defines various 2D shapes and their associated operations. It includes initialization, setting, and getting functions for shape properties, as well as functions to manipulate shapes such as moving them based on joystick input.

## Initialization Functions

### `void shape_init(Shape* shape)`
Initializes a `Shape` with default values.

**Parameters:**
- `shape`: A pointer to the `Shape` structure to be initialized.

### `void circle_init(Shape* circle, Point origin, float scale, float lod, color_t fillColor)`
Initializes a `circle` shape with specific parameters.

**Parameters:**
- `circle`: A pointer to the `Shape` structure to be initialized.
- `origin`: The center point of the circle.
- `scale`: The scale factor for the circle.
- `lod`: The level of detail for the circle.
- `fillColor`: The fill color of the circle.

### `void fan_init(Shape* fan, Point origin, float scale, int segments, color_t fillColor)`
Initializes a `fan` shape with specific parameters.

**Parameters:**
- `fan`: A pointer to the `Shape` structure to be initialized.
- `origin`: The center point of the fan.
- `scale`: The scale factor for the fan.
- `segments`: The number of segments in the fan.
- `fillColor`: The fill color of the fan.

### `void fan2_init(Shape* fan, Point origin, float scaleX, float scaleY, int segments, color_t fillColor)`
Initializes a `fan` shape with different scales for X and Y.

**Parameters:**
- `fan`: A pointer to the `Shape` structure to be initialized.
- `origin`: The center point of the fan.
- `scaleX`: The scale factor for the X-axis.
- `scaleY`: The scale factor for the Y-axis.
- `segments`: The number of segments in the fan.
- `fillColor`: The fill color of the fan.

### `void strip_init(Shape* strip, Point origin, float scaleX, float scaleY, float thickness, int segments, color_t fillColor)`
Initializes a `strip` shape with specific parameters.

**Parameters:**
- `strip`: A pointer to the `Shape` structure to be initialized.
- `origin`: The center point of the strip.
- `scaleX`: The scale factor for the X-axis.
- `scaleY`: The scale factor for the Y-axis.
- `thickness`: The thickness of the strip.
- `segments`: The number of segments in the strip.
- `fillColor`: The fill color of the strip.

## Common Functions for Shapes

### `void set_points(Shape* shape, PointArray* points)`
Sets the points for a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `points`: A pointer to the `PointArray` structure containing the points.

### `PointArray* get_points(Shape* shape)`
Returns the points of a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.

**Returns:**
- A pointer to the `PointArray` structure containing the points of the shape.

### `void set_thickness(Shape* shape, float thickness)`
Sets the thickness for a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `thickness`: The thickness to be set.

### `float get_thickness(const Shape* shape)`
Gets the thickness of a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.

**Returns:**
- The thickness of the shape.

### `void set_scaleX(Shape* shape, float scaleX)`
Sets the X scale for a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `scaleX`: The scale factor for the X-axis.

### `float get_scaleX(const Shape* shape)`
Gets the X scale of a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.

**Returns:**
- The X scale of the shape.

### `void set_scaleY(Shape* shape, float scaleY)`
Sets the Y scale for a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `scaleY`: The scale factor for the Y-axis.

### `float get_scaleY(const Shape* shape)`
Gets the Y scale of a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.

**Returns:**
- The Y scale of the shape.

### `void set_center(Shape* shape, Point center)`
Sets the center point for a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `center`: The center point to be set.

### `Point get_center(const Shape* shape)`
Gets the center point of a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.

**Returns:**
- The center point of the shape.

### `void set_segments(Shape* shape, int segments)`
Sets the number of segments for a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `segments`: The number of segments to be set.

### `int get_segments(const Shape* shape)`
Gets the number of segments of a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.

**Returns:**
- The number of segments of the shape.

### `void set_lod(Shape* shape, float lod)`
Sets the level of detail for a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `lod`: The level of detail to be set.

### `float get_lod(const Shape* shape)`
Gets the level of detail of a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.

**Returns:**
- The level of detail of the shape.

### `void set_fill_color(Shape* shape, color_t fillColor)`
Sets the fill color for a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `fillColor`: The fill color to be set.

### `color_t get_fill_color(const Shape* shape)`
Gets the fill color of a shape.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.

**Returns:**
- The fill color of the shape.

## Movement Function

### `void resolve(Shape* shape, float stickX, float stickY)`
Moves a shape around the screen using joystick input.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
- `stickX`: The X input from the joystick.
- `stickY`: The Y input from the joystick.

## Destruction Function

### `void destroy(Shape* shape)`
Frees the memory allocated for the shape's points.

**Parameters:**
- `shape`: A pointer to the `Shape` structure.
