#include <libdragon.h>
#include "rspq/vec_gfx.h"
#include "point.h"

// Constructors
Point point_new(float x, float y) {
    Point p = { x, y };
    return p;
}

Point point_default() {
    return point_new(0, 0);
}

// Operations
Point point_subtract(const Point* p1, const Point* p2) {
    return point_new(p1->x - p2->x, p1->y - p2->y);
}

Point point_add(const Point* p1, const Point* p2) {
    return point_new(p1->x + p2->x, p1->y + p2->y);
}

Point point_multiply(const Point* p, float scalar) {
    return point_new(p->x * scalar, p->y * scalar);
}

Point point_normalized(const Point* p) {
    float magnitude = sqrtf(p->x * p->x + p->y * p->y);
    if (magnitude == 0.0f) {
        return (Point){0, 0}; // Return a zero vector if the magnitude is zero
    }
    return (Point){p->x / magnitude, p->y / magnitude};
}

// Member functions
void point_add_in_place(Point* p, const Point* v) {
    p->x += v->x;
    p->y += v->y;
}

Point point_sum(const Point* v1, const Point* v2) {
    return point_add(v1, v2);
}

Point point_sub(const Point* v1, const Point* v2) {
    return point_subtract(v1, v2);
}

float point_heading(Point p) {
    return fm_atan2f(p.y, p.x);
}

Point point_from_angle(float angle) {
    return point_new(fm_cosf(angle), fm_sinf(angle));
}

float point_magnitude(const Point* p) {
    return sqrtf(p->x * p->x + p->y * p->y);
}

void point_normalize(Point* p) {
    float length = point_magnitude(p);
    if(length != 0) {
        p->x /= length;
        p->y /= length;
    }
}

Point point_set_mag(Point* p, float newMag) {
    Point normalized = point_normalized(p);
    return point_multiply(&normalized, newMag);
}

Point point_copy(const Point* p) {
    return point_new(p->x, p->y);
}

Point point_scale(const Point* center, const Point* point, float scale) {
    Point direction = point_subtract(point, center);
    Point scaled = point_multiply(&direction, scale);
    return point_add(center, &scaled);
}

void point_scale_batch(const Point* centers, const Point* points, float* scales, Point* scaled_points, uint32_t count) {
    // Ensure batch size does not exceed the allocated space
    if (count > MAX_BATCH_SIZE) {
        // Handle error or batch in chunks
        return;
    }

    vec_slot_t input_vecs[MAX_BATCH_SIZE];
    vec_slot_t output_vecs[MAX_BATCH_SIZE];
    vec_mtx_t scale_mtx;
    
    // Prepare a single scale matrix for all points
    mtx4x4_t scale_mat;
    matrix_scale(&scale_mat, scales[0], scales[0], 1.0f); // Use the first scale for simplicity
    floats_to_vectors(scale_mtx.c, scale_mat.m[0], 16);

    // Convert points to vectors and apply scaling
    for (uint32_t i = 0; i < count; ++i) {
        Point direction = point_subtract(&points[i], &centers[i]);
        floats_to_vectors(&input_vecs[i], (float*)&direction, 2);
    }

    // Apply transformation in batch
    apply_transformation(input_vecs, output_vecs, &scale_mtx, count);

    // Convert back to float and store results
    for (uint32_t i = 0; i < count; ++i) {
        float output_floats[2];
        vectors_to_floats(output_floats, &output_vecs[i], 2);
        scaled_points[i] = point_add(&centers[i], (Point*)output_floats);
    }
}

Point point_translate(Point p, float dx, float dy) {
    return point_new(p.x + dx, p.y + dy);
}

void point_rotate(Point* p, const Point* center, float angle) {
    float s = fm_sinf(angle);
    float c = fm_cosf(angle);
    
    p->x -= center->x;
    p->y -= center->y;

    float xnew = p->x * c - p->y * s;
    float ynew = p->x * s + p->y * c;

    p->x = xnew + center->x;
    p->y = ynew + center->y;
}


void point_rotate_batch(Point* points, const Point* center, float* angles, int count) {
    // Allocate space for vectors and results
    vec_slot_t input_vecs[count];
    vec_slot_t output_vecs[count];
    vec_mtx_t matrices[MAX_BATCH_SIZE];

    // Prepare the rotation matrices for each point
    for ( int i = 0; i < count; ++i) {
        mtx4x4_t rot_mat;
        matrix_rotate_z(&rot_mat, angles[i]); // Rotate by specific angle
        floats_to_vectors(matrices[i].c, rot_mat.m[0], 16); // Convert the rotation matrix to vec_mtx_t
    }


    // Prepare vectors
    for (int i = 0; i < count; i++) {
        Point temp = point_subtract(&points[i], center);
        floats_to_vectors(&input_vecs[i], (float*)&temp, 2);
    }

    // Apply rotation transformation
    for (int i = 0; i < count; ++i) {
        apply_transformation(&input_vecs[i], &output_vecs[i], &matrices[i], 1);
    }

    // Convert back to float and update points
    for (int i = 0; i < count; ++i) {
        float output_floats[2];
        vectors_to_floats(output_floats, &output_vecs[i], 2);
        points[i] = point_add(&center[i], (Point*)output_floats);
    }
}

Point point_transform(const Point* point, float angle, float width) {
    Point rotated = point_from_angle(angle);
    Point scaled = point_multiply(&rotated, width);
    return point_add(point, &scaled);
}

void point_transform_4x4_batch(Point* points, const mtx4x4_t *mat, int count) {
    // Allocate space for vectors and results
    vec_slot_t input_vecs[count];
    vec_slot_t output_vecs[count];
    vec_mtx_t transform_mtx;
    
    // Convert transformation matrix to fixed point format
    floats_to_vectors(transform_mtx.c, mat->m[0], 16);

    // Prepare vectors
    for (int i = 0; i < count; i++) {
        floats_to_vectors(&input_vecs[i], (float*)&points[i], 2);
    }

    // Apply transformation to all points
    apply_transformation(input_vecs, output_vecs, &transform_mtx, count);

    // Convert back to float and update points
    float output_floats[count * 2];
    vectors_to_floats(output_floats, output_vecs, count * 2);

    for (int i = 0; i < count; i++) {
        points[i] = point_new(output_floats[i * 2], output_floats[i * 2 + 1]);
    }
}

float point_cross(const Point* p1, const Point* p2) {
    return p1->x * p2->y - p1->y * p2->x;
}

float point_dot(const Point* p1, const Point* p2) {
    return p1->x * p2->x + p1->y * p2->y;
}

float point_epsilon_test(const Point* A, const Point* B, const Point* C) {
    float epsilon = 1e-6f;
    Point AB = point_subtract(B, A);
    Point AC = point_subtract(C, A);
    float cross = point_cross(&AB, &AC);
    return fabsf(cross) < epsilon;
}

bool point_in_triangle(const Point* P, const Point* A, const Point* B, const Point* C) {
    Point v0 = point_subtract(C, A);
    Point v1 = point_subtract(B, A);
    Point v2 = point_subtract(P, A);

    float dot00 = point_dot(&v0, &v0);
    float dot01 = point_dot(&v0, &v1);
    float dot02 = point_dot(&v0, &v2);
    float dot11 = point_dot(&v1, &v1);
    float dot12 = point_dot(&v1, &v2);

    float invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0) && (v >= 0) && (u + v < 1);
}

void point_move(Point* p, float stickX, float stickY) {
    p->x += stickX;
    p->y += stickY;
}

void rotate_line_point(Point* p, const Point* center, float cos_angle, float sin_angle) {
    float tx = p->x - center->x;
    float ty = p->y - center->y;
    p->x = center->x + (tx * cos_angle - ty * sin_angle);
    p->y = center->y + (tx * sin_angle + ty * cos_angle);
}

Point point_lerp(Point* p1, Point* p2, float t) {
    Point result;
    result.x = p1->x * (1.0f - t) + p2->x * t;
    result.y = p1->y * (1.0f - t) + p2->y * t;
    return result;
}

// Function to initialize a PointArray
void init_point_array(PointArray* array) {
    if (array) {
        array->points = (Point*)malloc(sizeof(Point));
        if (array->points) {
            array->count = 0;
        } else {
            array->count = 0;
        }
    }
}

// Function to initialize a PointArray from existing points
void init_point_array_from_points(PointArray* array, Point* points, size_t count) {
    array->points = (Point*)malloc(sizeof(Point) * count);
    if (array->points == NULL) {
        // Handle memory allocation failure
        debugf("Point allocation failed\n");
        return;
    }
    memcpy(array->points, points, sizeof(Point) * count);
    array->count = count;
}

// Function to add a point to a PointArray
void add_point(PointArray* array, float x, float y) {
    Point* new_points = (Point*)realloc(array->points, sizeof(Point) * (array->count + 1));
    if (new_points == NULL) {
        debugf("Point reallocation failed\n");
        return;
    }
    array->points = new_points;
    array->points[array->count].x = x;
    array->points[array->count].y = y;
    array->count++;
}

// Function to add an existing point to the PointArray
void add_existing_point(PointArray* array, Point p) {
    Point* new_points = (Point*)realloc(array->points, sizeof(Point) * (array->count + 1));
    if (new_points == NULL) {
        debugf("Point reallocation failed\n");
        return;
    }
    array->points = new_points;
    array->points[array->count] = p;
    array->count++;
}

void calculate_array_center(const PointArray* points, Point* center) {
    center->x = 0.0f;
    center->y = 0.0f;

    for (size_t i = 0; i < points->count; ++i) {
        center->x += points->points[i].x;
        center->y += points->points[i].y;
    }

    center->x /= points->count;
    center->y /= points->count;
}


// Function to free a PointArray
void free_point_array(PointArray* array) {
    free_uncached(array);
}

void constrain_dist_rsp(Point* positions, Point* anchors, float* constraints, int count) {
    // Allocate memory for DMA transfers
    vec_slot_t* input_vectors = (vec_slot_t *)malloc_uncached(sizeof(vec_slot_t) * count);
    vec_slot_t* output_vectors = (vec_slot_t *)malloc_uncached(sizeof(vec_slot_t) * count);
    vec_mtx_t* matrices = (vec_mtx_t *)malloc_uncached(sizeof(vec_mtx_t) * count);

    memset(input_vectors, 0, sizeof(vec_slot_t) * count);
    memset(output_vectors, 0, sizeof(vec_slot_t) * count);
    memset(matrices, 0, sizeof(vec_mtx_t) * count);

    // Initialize vectors and matrices
    for (int i = 0; i < count; i++) {
        mtx4x4_t translation, scale;
        matrix_translate(&translation, -anchors[i].x, -anchors[i].y, 0);
        matrix_scale(&scale, constraints[i], constraints[i], 1);

        // Set up points
        vec4_t vectors[2] = {
            {{ positions[i].x, positions[i].y, 0, 1 }},
            {{ 0, 0, 0, 1 }}
        };

        // Convert vectors to fixed point format required by the overlay
        floats_to_vectors(&input_vectors[i], (float*)&vectors[0], 2);

        // Convert matrices to fixed point format required by the overlay
        floats_to_vectors(matrices[i].c, translation.m[0], 16);
        floats_to_vectors(matrices[i].c, scale.m[0], 16);
    }

    // Apply transformations in batch
    apply_transformation_batch(input_vectors, output_vectors, matrices, count);

    // Store results back into positions
    float* output_floats = (float*)malloc(NUM_VECTOR_SLOTS * sizeof(float));
    for (int i = 0; i < count; i++) {
        vectors_to_floats(output_floats, &output_vectors[i], 2);
        positions[i] = (Point){ output_floats[0], output_floats[1] };
    }
    
    free(output_floats);
    free_uncached(matrices);
    free_uncached(output_vectors);
    free_uncached(input_vectors);
}
