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
    float c = cosf(angle);
    float s = sinf(angle);

    *dest = (mtx4x4_t){ .m={
        {1.f, 0.f, 0.f, 0.f},
        {0.f, c,   s,   0.f},
        {0.f, -s,  c,   0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
}

void matrix_rotate_y(mtx4x4_t *dest, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    *dest = (mtx4x4_t){ .m={
        {c,   0.f, -s,  0.f},
        {0.f, 1.f, 0.f, 0.f},
        {s,   0.f, c,   0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
}

void matrix_rotate_z(mtx4x4_t *dest, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    *dest = (mtx4x4_t){ .m={
        {c,   s,   0.f, 0.f},
        {-s,  c,   0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
    }};
}

vec4_t vec4_transform(const mtx4x4_t *mat, const vec4_t *vec) {
    return (vec4_t){
        .v[0] = mat->m[0][0] * vec->v[0] + mat->m[0][1] * vec->v[1] + mat->m[0][2] * vec->v[2] + mat->m[0][3] * vec->v[3],
        .v[1] = mat->m[1][0] * vec->v[0] + mat->m[1][1] * vec->v[1] + mat->m[1][2] * vec->v[2] + mat->m[1][3] * vec->v[3],
        .v[2] = mat->m[2][0] * vec->v[0] + mat->m[2][1] * vec->v[1] + mat->m[2][2] * vec->v[2] + mat->m[2][3] * vec->v[3],
        .v[3] = mat->m[3][0] * vec->v[0] + mat->m[3][1] * vec->v[1] + mat->m[3][2] * vec->v[2] + mat->m[3][3] * vec->v[3],
    };
}
