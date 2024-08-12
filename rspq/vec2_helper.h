/**
 * Some helper structs and functions for common vector/matrix operations.
 */

#ifndef VEC2_HELPER_H
#define VEC2_HELPER_H

#include <math.h>
#include "vector_helper.h"

typedef struct {
    float v[2];
} vec2_t;

typedef struct {
    float m[2][2];
} mtx2x2_t;

vec4_t vec2_to_vec4(const vec2_t* v1, const vec2_t* v2) {
    vec4_t result;
    result.v[0] = v1->v[0];
    result.v[1] = v1->v[1];
    result.v[2] = v2->v[0];
    result.v[3] = v2->v[1];
    return result;
}

void vec4_to_vec2(const vec4_t* v4, vec2_t* v1, vec2_t* v2) {
    if (v1) {
        v1->v[0] = v4->v[0];
        v1->v[1] = v4->v[1];
    }
    if (v2) {
        v2->v[0] = v4->v[2];
        v2->v[1] = v4->v[3];
    }
}

void vec2_mtx_id(mtx2x2_t *dest) {
    *dest = (mtx2x2_t){ .m={
        {1.f, 0.f},
        {0.f, 1.f},
    }};
}

vec2_t vec2_add(const vec2_t* v1, const vec2_t* v2) {
    return (vec2_t){
        .v[0] = v1->v[0] + v2->v[0],
        .v[1] = v1->v[1] + v2->v[1],
    };
}

vec2_t vec2_subtract(const vec2_t* v1, const vec2_t* v2) {
    return (vec2_t){
        .v[0] = v1->v[0] - v2->v[0],
        .v[1] = v1->v[1] - v2->v[1],
    };
}

vec2_t vec2_multiply(const vec2_t* v, float scalar) {
    return (vec2_t){
        .v[0] = v->v[0] * scalar,
        .v[1] = v->v[1] * scalar,
    };
}

float vec2_heading(const vec2_t* v) {
    return atan2f(v->v[1], v->v[0]);
}

vec2_t vec2_from_angle(float angle) {
    return (vec2_t){
        .v[0] = cosf(angle),
        .v[1] = sinf(angle),
    };
}

vec2_t vec2_normalized(const vec2_t* v) {
    float magnitude = sqrtf(v->v[0] * v->v[0] + v->v[1] * v->v[1]);
    if (magnitude == 0.0f) {
        return (vec2_t){ .v[0] = 0.0f, .v[1] = 0.0f }; // Return zero vector if magnitude is zero
    }
    return (vec2_t){
        .v[0] = v->v[0] / magnitude,
        .v[1] = v->v[1] / magnitude,
    };
}

vec2_t vec2_rotate(const vec2_t* v, const vec2_t* center, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);

    vec2_t translated = vec2_subtract(v, center);

    return (vec2_t){
        .v[0] = translated.v[0] * c - translated.v[1] * s + center->v[0],
        .v[1] = translated.v[0] * s + translated.v[1] * c + center->v[1],
    };
}

vec2_t vec2_lerp(const vec2_t* v1, const vec2_t* v2, float t) {
    return (vec2_t){
        .v[0] = v1->v[0] * (1.0f - t) + v2->v[0] * t,
        .v[1] = v1->v[1] * (1.0f - t) + v2->v[1] * t,
    };
}

float vec2_cross(const vec2_t* v1, const vec2_t* v2) {
    return v1->v[0] * v2->v[1] - v1->v[1] * v2->v[0];
}

float vec2_dot(const vec2_t* v1, const vec2_t* v2) {
    return v1->v[0] * v2->v[0] + v1->v[1] * v2->v[1];
}

float vec2_epsilon_test(const vec2_t* A, const vec2_t* B, const vec2_t* C) {
    float epsilon = 1e-6f;
    vec2_t AB = vec2_subtract(B, A);
    vec2_t AC = vec2_subtract(C, A);
    float cross = vec2_cross(&AB, &AC);
    return fabsf(cross) < epsilon;
}

bool vec2_in_triangle(const vec2_t* P, const vec2_t* A, const vec2_t* B, const vec2_t* C) {
    vec2_t v0 = vec2_subtract(C, A);
    vec2_t v1 = vec2_subtract(B, A);
    vec2_t v2 = vec2_subtract(P, A);

    float dot00 = vec2_dot(&v0, &v0);
    float dot01 = vec2_dot(&v0, &v1);
    float dot02 = vec2_dot(&v0, &v2);
    float dot11 = vec2_dot(&v1, &v1);
    float dot12 = vec2_dot(&v1, &v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0) && (v >= 0) && (u + v < 1);
}

vec2_t vec2_transform(const mtx2x2_t *mat, const vec2_t *vec) {
    vec2_t result;
    result.v[0] = mat->m[0][0] * vec->v[0] + mat->m[0][1] * vec->v[1];
    result.v[1] = mat->m[1][0] * vec->v[0] + mat->m[1][1] * vec->v[1];
    return result;
}

void vec2_mtx_scale(mtx2x2_t *dest, float x, float y) {
    *dest = (mtx2x2_t){ .m={
        {x,   0.f},
        {0.f, y},
    }};
}

void vec2_mtx_rotate(mtx2x2_t *dest, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);

    *dest = (mtx2x2_t){ .m={
        {c,   -s},
        {s,   c},
    }};
}

vec2_t vec2_translate(const vec2_t* vec, const mtx4x4_t* trans) {
    vec4_t v4 = vec2_to_vec4(vec, &(vec2_t){ .v[0] = 0.0f, .v[1] = 0.0f });
    vec4_t result = vec4_transform(trans, &v4);
    vec2_t result_vec;
    vec4_to_vec2(&result, &result_vec, NULL);
    return result_vec;
}

#endif
