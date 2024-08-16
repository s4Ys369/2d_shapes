#ifndef RDPQ_FAN_H
#define RDPQ_FAN_H

#include <libdragon.h>

// ====~ Required functions from RDPQ - start ~==== //

/** @brief Round n up to the next multiple of d */
#define ROUND_UP(n, d) ({ \
	typeof(n) _n = n; typeof(d) _d = d; \
	(((_n) + (_d) - 1) / (_d) * (_d)); \
})

/** @brief Converts a float to a s16.16 fixed point number */
static int32_t float_to_s16_16(float f)
{
    // Currently the float must be clamped to this range because
    // otherwise the trunc.w.s instruction can potentially trigger
    // an unimplemented operation exception due to integer overflow.
    // TODO: maybe handle the exception? Clamp the value in the exception handler?
    if (f >= 32768.f) {
        return 0x7FFFFFFF;
    }

    if (f < -32768.f) {
        return 0x80000000;
    }

    return floor(f * 65536.f);
}

// ====~ Required functions from RDPQ - end ~==== //

// ============~ Fan Overlay - start ~=========== //

#define RDPQ_CMD_FAN_ADD 0x00
extern uint32_t fan_add_id;

// ============~ Fan Overlay - end ~============ //

// ================~ Fan API ~================== //

typedef enum {
    TRI_DATA_NEXT,
    TRI_DATA_LAST,
    TRI_DATA_CENTER
} TRI_DATA_SLOT;

typedef struct rdpq_fan_s {
    float cv[4]; // Center vertex
    float v1[4]; // First vertex of the fan after the center
    float pv[4]; // Holds the previous vertex
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

// This is the higher level call to add to TRI_DATA
void rdpq_add_tri_data(const float* vtx, int triDataSlot) {

    if (!vtx) {
        debugf("rdpq_add_tri_data: Invalid arguments\n");
        return;
    }

    const int TRI_DATA_LEN = ROUND_UP((2+1+1+3)*4, 16);

    // Follow the normal steps for getting the vertex data
    if (state->fmt->shade_offset >= 0) state->cmd_id |= 0x4;
    if (state->fmt->tex_offset >= 0)   state->cmd_id |= 0x2;
    if (state->fmt->z_offset >= 0)     state->cmd_id |= 0x1;


    int16_t x = floorf(vtx[state->fmt->pos_offset + 0] * 4.0f);
    int16_t y = floorf(vtx[state->fmt->pos_offset + 1] * 4.0f);

    int16_t z = 0;
    if (state->fmt->z_offset >= 0) {
        z = vtx[state->fmt->z_offset + 0] * 0x7FFF;
    }
    int32_t rgba = 0;
    if (state->fmt->shade_offset >= 0) {
        const float *v_shade = vtx;
        uint32_t r = v_shade[state->fmt->shade_offset + 0] * 255.0;
        uint32_t g = v_shade[state->fmt->shade_offset + 1] * 255.0;
        uint32_t b = v_shade[state->fmt->shade_offset + 2] * 255.0;
        uint32_t a = v_shade[state->fmt->shade_offset + 3] * 255.0;
        rgba = (r << 24) | (g << 16) | (b << 8) | a;
    }
    int16_t s = 0, t = 0;
    int32_t w = 0, inv_w = 0;
    if (state->fmt->tex_offset >= 0) {
        s = vtx[state->fmt->tex_offset + 0] * 32.0f;
        t = vtx[state->fmt->tex_offset + 1] * 32.0f;
        w = float_to_s16_16(1.0f / vtx[state->fmt->tex_offset + 2]);
        inv_w = float_to_s16_16(vtx[state->fmt->tex_offset + 2]);
    }

    // Write vertex one at a time
    rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE_DATA,
        TRI_DATA_LEN * triDataSlot, 
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

    memcpy(state->cv, cv, sizeof(float) * 4);

    state->fmt = fmt;
    state->cmd_id = RDPQ_CMD_TRI;
    state->v1Added = false;

    rdpq_add_tri_data(cv, TRI_DATA_CENTER);

}

// This is the higher level CPU implementation of the RSP code
void rdpq_fan_add_new_triangle_cpu(const float* pv, const float* v) {
    // Move last position and store current vertex
    rdpq_add_tri_data(v, TRI_DATA_NEXT);
    rdpq_add_tri_data(pv, TRI_DATA_LAST);

    // Store current vertex and increment counter
    memcpy(state->pv, v, sizeof(float) * 4);
    state->vtxCount++;
}

// This is the highest level call to handle constructing the fan
void rdpq_fan_add_vertex(const float* v) {

    if (state->vtxCount == 0) {
        // Store the first vertex
        memcpy(state->v1, v, sizeof(float) * 4);
        state->v1Added = true;
        state->vtxCount++;
        rdpq_add_tri_data(state->v1, TRI_DATA_LAST);
        
    } else if (state->vtxCount >= 1) {

        if (state->vtxCount == 1){
            rdpq_add_tri_data(v, TRI_DATA_NEXT);

            // Store first vertex as previous point
            memcpy(state->pv, state->v1, sizeof(float) * 4);
            state->vtxCount++;
        }


        // Move last position and store current vertex
        rdpq_fan_add_new_triangle_cpu(state->pv, v);

        // Until i get the overlay working, i have to call this for each new vertex
        rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE, 
            0xC000 | (state->cmd_id << 8) | 
            (state->fmt->tex_mipmaps ? (state->fmt->tex_mipmaps - 1) << 3 : 0) | 
            (state->fmt->tex_tile & 7));

    }


}

// Function to close the fan and complete the shape
void rdpq_fan_end() {

    rdpq_fan_add_vertex(state->v1);

    free_uncached(state);
    state = NULL;
}


#endif // RDPQ_FAN_H