// vector_helper.h
#ifndef VEC_GFX_H
#define VEC_GFX_H

#include <libdragon.h>
#include "vector_helper.h"

#define NUM_VECTOR_SLOTS  16
#define NUM_VECTORS       (NUM_VECTOR_SLOTS*2)
#define NUM_MATRICES      4
#define MTX_SLOT          30

extern vec4_t vectors[NUM_VECTORS];
extern mtx4x4_t identity, scale, rotation, translation;

extern vec_slot_t *input_vectors;
extern vec_slot_t *output_vectors;
extern vec_mtx_t *matrices;

extern rspq_block_t *transform_vectors_block;

void vec_gfx_alloc();

void vec_gfx_destroy();

// Load vectors and apply a matrix transformation
void apply_transformation(vec_slot_t* input_vectors, vec_slot_t* output_vectors, vec_mtx_t* matrix, uint32_t num_vectors);

// Function to apply a specific transformation to vectors
void transform_vectors(vec_slot_t* input_vectors, vec_slot_t* output_vectors, vec_mtx_t* matrix, uint32_t num_vectors);

void apply_transformation_batch(vec_slot_t* input_vectors, vec_slot_t* output_vectors, vec_mtx_t* matrices, uint32_t num_vectors);

#endif // VEC_GFX_H
