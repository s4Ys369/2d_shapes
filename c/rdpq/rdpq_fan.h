#ifndef RDPQ_FAN_H
#define RDPQ_FAN_H

#include <libdragon.h>
#include "rdpq_common.h"

// ============~ Fan Overlay - start ~=========== //

#define RDPQ_CMD_FAN_ADD 0x00
extern uint32_t fan_add_id;

// ============~ Fan Overlay - end ~============ //

// ================~ Fan API ~================== //

typedef struct rdpq_fan_s {
    float cv[8]; // Center vertex
    float v1[8]; // First vertex of the fan after the center
    float pv[8]; // Holds the previous vertex
    const rdpq_trifmt_t* fmt; // Triangle format
    uint32_t cmd_id; // RPDQ command ID
    bool v1Added; // Check to see any vertices have been added after rdpq_fan_begin
    int vtxCount; // Counts number of vertices
} rdpq_fan_t;

rdpq_fan_t* state; // TODO: This will eventually be a argument in the higher level functions to allow for multiple fans

// This is the higher level call for RSP code
void rdpq_fan_add_new(const rdpq_trifmt_t *fmt, const float* vtx) {

    // Follow the normal steps for getting the vertex data
    uint32_t cmd_id = RDPQ_CMD_TRI;
    if (fmt->shade_offset >= 0) cmd_id |= 0x4;
    if (fmt->tex_offset >= 0)   cmd_id |= 0x2;
    if (fmt->z_offset >= 0)     cmd_id |= 0x1;


    int16_t x = floorf(vtx[fmt->pos_offset + 0] * 4.0f);
    int16_t y = floorf(vtx[fmt->pos_offset + 1] * 4.0f);

    int16_t z = 0;
    if (state->fmt->z_offset >= 0) {
        z = vtx[state->fmt->z_offset + 0] * 0x7FFF;
    }
    int32_t rgba = 0;
    if (state->fmt->shade_offset >= 0) {
        const float *v_shade = vtx;
        uint32_t r = v_shade[fmt->shade_offset + 0] * 255.0;
        uint32_t g = v_shade[fmt->shade_offset + 1] * 255.0;
        uint32_t b = v_shade[fmt->shade_offset + 2] * 255.0;
        uint32_t a = v_shade[fmt->shade_offset + 3] * 255.0;
        rgba = (r << 24) | (g << 16) | (b << 8) | a;
    }
    int16_t s = 0, t = 0;
    int32_t w = 0, inv_w = 0;
    if (fmt->tex_offset >= 0) {
        s = vtx[fmt->tex_offset + 0] * 32.0f;
        t = vtx[fmt->tex_offset + 1] * 32.0f;
        w = float_to_s16_16(1.0f / vtx[fmt->tex_offset + 2]);
        inv_w = float_to_s16_16(vtx[fmt->tex_offset + 2]);
    }

    // Write vertex and send tri async using overlay cmd
    rspq_write(fan_add_id, RDPQ_CMD_FAN_ADD,
        TRI_DATA_NEXT, 
        (x << 16) | (y & 0xFFFF), 
        (z << 16), 
        rgba, 
        (s << 16) | (t & 0xFFFF), 
        w,
        inv_w);

}

rdpq_fan_t* rdpq_fan_init() {
    state = (rdpq_fan_t*)malloc_uncached(sizeof(rdpq_fan_t));
    memset(state, 0, sizeof(rdpq_fan_t)); // Initialize all values to 0
    return state;
}

// Function to initialize the fan drawing
void rdpq_fan_begin(const rdpq_trifmt_t *fmt, const float *cv) {

    if (!fmt || !cv) {
        debugf("rdpq_fan_begin: Invalid arguments\n");
        return;
    }

    state = rdpq_fan_init();
    if (!state) {
        debugf("rdpq_fan_begin: Memory allocation failed\n");
        return;
    }

    memcpy(state->cv, cv, sizeof(float) * 8);

    state->fmt = fmt;
    state->cmd_id = RDPQ_CMD_TRI;
    state->v1Added = false;

    rdpq_add_tri_data(fmt, cv, TRI_DATA_CENTER);

}

// This is the higher level CPU implementation of the RSP code
void rdpq_fan_add_new_triangle_cpu(const float* pv, const float* v) {
    // Move last position and store current vertex
    rdpq_add_tri_data(state->fmt, v, TRI_DATA_NEXT);
    rdpq_add_tri_data(state->fmt, pv, TRI_DATA_LAST);

    // Store current vertex and increment counter
    memcpy(state->pv, v, sizeof(float) * 8);
    state->vtxCount++;
}

// This is the highest level call to handle constructing the fan
void rdpq_fan_add_vertex(const float* v) {

    if (state->vtxCount == 0) {
        // Store the first vertex
        memcpy(state->v1, v, sizeof(float) * 8);
        state->v1Added = true;
        state->vtxCount++;
        rdpq_add_tri_data(state->fmt, state->v1, TRI_DATA_LAST);
        
    } else if (state->vtxCount >= 1) {

        if (state->vtxCount == 1){
            rdpq_add_tri_data(state->fmt, v, TRI_DATA_NEXT);


            // Store first vertex as previous point
            memcpy(state->pv, state->v1, sizeof(float) * 8);
            state->vtxCount++;
        }


        // Move last position and store current vertex
        rdpq_fan_add_new_triangle_cpu(state->pv, v);

        rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE, 
                0xC000 | (state->cmd_id << 8) | 
                (state->fmt->tex_mipmaps ? (state->fmt->tex_mipmaps-1) << 3 : 0) | 
                (state->fmt->tex_tile & 7));


    }


}

void rdpq_fan_destroy(){
    free_uncached(state);
    state = NULL;
}

// Function to close the fan and complete the shape
void rdpq_fan_end() {

    rdpq_fan_add_vertex(state->v1);
    rdpq_fan_destroy();
    
}


#endif // RDPQ_FAN_H