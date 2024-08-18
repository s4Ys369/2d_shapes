#ifndef RDPQ_STRIP_H
#define RDPQ_STRIP_H

#include <libdragon.h>

// ================~ Strip API ~================== //

typedef struct rdpq_strip_s {
    float vA[8]; 
    float vC[8]; 
    const rdpq_trifmt_t* fmt; // Triangle format
    uint32_t cmd_id; // RPDQ command ID
} rdpq_strip_t;

rdpq_strip_t* stripState; // TODO: This will eventually be a argument in the higher level functions to allow for multiple fans

rdpq_strip_t* rdpq_strip_init(const rdpq_trifmt_t *fmt) {
    stripState = (rdpq_strip_t*)malloc_uncached(sizeof(rdpq_strip_t));
    memset(stripState, 0, sizeof(rdpq_strip_t)); // Initialize all values to 0
    stripState->fmt = fmt;
    stripState->cmd_id = RDPQ_CMD_TRI;
    if (!stripState) {
        debugf("rdpq_strip_init: Memory allocation failed\n");
        return NULL;
    } else {
        return stripState;
    }
}

// Function to initialize the strip drawing
void rdpq_strip_begin(const rdpq_trifmt_t *fmt, const float *v1, const float *v2, const float *v3) {
    rdpq_strip_init(fmt);

    rdpq_add_tri_data(fmt, v1, TRI_DATA_A);
    rdpq_add_tri_data(fmt, v2, TRI_DATA_B);
    rdpq_add_tri_data(fmt, v3, TRI_DATA_C);
    rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE, 
                0xC000 | (stripState->cmd_id << 8) | 
                (stripState->fmt->tex_mipmaps ? (stripState->fmt->tex_mipmaps-1) << 3 : 0) | 
                (stripState->fmt->tex_tile & 7));

    memcpy(stripState->vA, v2, sizeof(float) * 8);

}

// This is the highest level call to handle adding to the strip
void rdpq_strip_add_segment(const float* v) {
    rdpq_add_tri_data(stripState->fmt, stripState->vA, TRI_DATA_A);
    rdpq_add_tri_data(stripState->fmt, v, TRI_DATA_B);
    rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE, 
                0xC000 | (stripState->cmd_id << 8) | 
                (stripState->fmt->tex_mipmaps ? (stripState->fmt->tex_mipmaps-1) << 3 : 0) | 
                (stripState->fmt->tex_tile & 7));

    memcpy(stripState->vC, stripState->vA, sizeof(float) * 8);
    memcpy(stripState->vA, v, sizeof(float) * 8);

}

void rdpq_strip_destroy(){
    free_uncached(stripState);
    stripState = NULL;
}


#endif // RDPQ_STRIP_H