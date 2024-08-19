// vector_helper.h
#ifndef VECTOR_HELPER_H
#define VECTOR_HELPER_H

typedef struct {
    float v[4];
} vec4_t;

typedef struct {
    float m[4][4];
} mtx4x4_t;

// Function declarations
void matrix_identity(mtx4x4_t *mat);
void matrix_scale(mtx4x4_t *mat, float sx, float sy, float sz);
void matrix_rotate_x(mtx4x4_t *mat, float angle);
void matrix_rotate_y(mtx4x4_t *mat, float angle);
void matrix_rotate_z(mtx4x4_t *mat, float angle);
void matrix_translate(mtx4x4_t *mat, float tx, float ty, float tz);

vec4_t vec4_transform(const mtx4x4_t *mat, const vec4_t *vec);

#endif // VECTOR_HELPER_H
