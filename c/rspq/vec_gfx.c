#include <libdragon.h>
#include "vec_gfx.h"

vec4_t vectors[NUM_VECTORS];
mtx4x4_t identity, scale, rotation, translation;

vec_slot_t *input_vectors;
vec_slot_t *output_vectors;
vec_mtx_t *matrices;

rspq_block_t *transform_vectors_block;

void vec_gfx_alloc(){
    // Allocate memory for DMA transfers
    input_vectors = (vec_slot_t *)malloc_uncached_aligned(sizeof(vec_slot_t)*NUM_VECTOR_SLOTS, 16);
    output_vectors = (vec_slot_t *)malloc_uncached_aligned(sizeof(vec_slot_t)*NUM_VECTOR_SLOTS, 16);
    matrices = (vec_mtx_t *)malloc_uncached_aligned(sizeof(vec_mtx_t)*NUM_MATRICES, 16);

    memset(input_vectors, 0, sizeof(vec_slot_t)*NUM_VECTOR_SLOTS);
    memset(output_vectors, 0, sizeof(vec_slot_t)*NUM_VECTOR_SLOTS);
    memset(matrices, 0, sizeof(vec_mtx_t)*NUM_MATRICES);
}

void vec_gfx_destroy(){
    // Clean up
    if(transform_vectors_block)rspq_block_free(transform_vectors_block);
    free_uncached(matrices);
    free_uncached(output_vectors);
    free_uncached(input_vectors);
    transform_vectors_block=NULL;
    matrices=NULL;
    output_vectors=NULL;
    input_vectors=NULL;

}

// Load vectors and apply a matrix transformation
void apply_transformation(vec_slot_t* input_vectors, vec_slot_t* output_vectors, vec_mtx_t* matrix, uint32_t num_vectors) {
    vec_load(0, input_vectors, num_vectors);
    vec_load(MTX_SLOT, matrix[0].c, 16); // Load the transformation matrix
    for (uint32_t i = 0; i < num_vectors; i++) {
        vec_transform(i, MTX_SLOT, i);
    }
    vec_store(output_vectors, 0, num_vectors);
}

// Function to apply a specific transformation to vectors
void transform_vectors(vec_slot_t* input_vectors, vec_slot_t* output_vectors, vec_mtx_t* matrix, uint32_t num_vectors) {
    apply_transformation(input_vectors, output_vectors, matrix, num_vectors);
}

void apply_transformation_batch(vec_slot_t* input_vectors, vec_slot_t* output_vectors, vec_mtx_t* matrices, uint32_t num_vectors) {
    rspq_block_begin();
    vec_load(0, input_vectors, num_vectors); // Load all vectors at once

    for (uint32_t i = 0; i < num_vectors; ++i) {
        vec_load(MTX_SLOT + i, matrices[i].c, 16); // Load each transformation matrix
        vec_transform(i, MTX_SLOT + i, i); // Apply transformation
    }

    vec_store(output_vectors, 0, num_vectors); // Store all vectors at once
    rspq_block_end();
}
