/**
 * Some helper structs and functions for common vector/matrix operations.
 */
#include <math.h>
#include "vector_helper.h"

void matrix_identity(mtx4x4_t *dest)
{
    *dest = (mtx4x4_t){ .m={
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
}

void matrix_translate(mtx4x4_t *dest, float x, float y, float z)
{
    *dest = (mtx4x4_t){ .m={
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {x,   y,   z,   1.f},
    }};
}

void matrix_scale(mtx4x4_t *dest, float x, float y, float z)
{
    *dest = (mtx4x4_t){ .m={
        {x,   0.f, 0.f, 0.f},
        {0.f, y,   0.f, 0.f},
        {0.f, 0.f, z,   0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
}

void matrix_rotate_x(mtx4x4_t *dest, float angle)
{
    float c = fm_cosf(angle);
    float s = fm_sinf(angle);

    *dest = (mtx4x4_t){ .m={
        {1.f, 0.f, 0.f, 0.f},
        {0.f, c,   s,   0.f},
        {0.f, -s,  c,   0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
}

void matrix_rotate_y(mtx4x4_t *dest, float angle)
{
    float c = fm_cosf(angle);
    float s = fm_sinf(angle);

    *dest = (mtx4x4_t){ .m={
        {c,   0.f, -s,  0.f},
        {0.f, 1.f, 0.f, 0.f},
        {s,   0.f, c,   0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
}

void matrix_rotate_z(mtx4x4_t *dest, float angle)
{
    float c = fm_cosf(angle);
    float s = fm_sinf(angle);

    *dest = (mtx4x4_t){ .m={
        {c,   s,   0.f, 0.f},
        {-s,  c,   0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
}

// Multiply two 4x4 matrices
void matrix_multiply(const mtx4x4_t *a, const mtx4x4_t *b, mtx4x4_t *result) {
    // Initialize result matrix to zero
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result->m[i][j] = 0;
        }
    }
    
    // Perform matrix multiplication
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result->m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }
}

vec4_t vec4_transform(const mtx4x4_t *mat, const vec4_t *vec) {
    vec4_t result;

    result.v[0] = mat->m[0][0] * vec->v[0] + mat->m[0][1] * vec->v[1] + mat->m[0][2] * vec->v[2] + mat->m[0][3] * vec->v[3];
    result.v[1] = mat->m[1][0] * vec->v[0] + mat->m[1][1] * vec->v[1] + mat->m[1][2] * vec->v[2] + mat->m[1][3] * vec->v[3];
    result.v[2] = mat->m[2][0] * vec->v[0] + mat->m[2][1] * vec->v[1] + mat->m[2][2] * vec->v[2] + mat->m[2][3] * vec->v[3];
    result.v[3] = mat->m[3][0] * vec->v[0] + mat->m[3][1] * vec->v[1] + mat->m[3][2] * vec->v[2] + mat->m[3][3] * vec->v[3];

    return result;
}
