#ifndef RDPQ_FAN_H
#define RDPQ_FAN_H

#include <libdragon.h>

// I noticed cmd_id 0x20 through 0x23 aren't used. Intentional?
#define RDPQ_CMD_FAN_ADD 0x20

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

typedef struct rdpq_fan_s {
    float cv[2]; // Center vertex
    float v1[2]; // First vertex of the fan after the center
    float pv[2]; // Holds the previous vertex
    const rdpq_trifmt_t* fmt; // Triangle format
    uint32_t cmd_id; // RPDQ command ID
    bool v1Added; // Check to see any vertices have been added after rdpq_fan_begin
    int vtxCount; // Counts number of vertices
} rdpq_fan_t;

rdpq_fan_t* state;

rdpq_fan_t* rdpq_fan_init() {
    state = (rdpq_fan_t*)malloc_uncached(sizeof(rdpq_fan_t));
    memset(state, 0, sizeof(rdpq_fan_t)); // Initialize all values to 0
    return state;
}

// Function to initialize the fan drawing
void rdpq_fan_begin(const rdpq_trifmt_t *fmt, const float *cv) {
    state = rdpq_fan_init();
    memcpy(state->cv, cv, sizeof(state->cv));
    state->v1Added = false;
    state->cmd_id = RDPQ_CMD_TRI;
    if (fmt->shade_offset >= 0) state->cmd_id |= 0x4;
    if (fmt->tex_offset >= 0)   state->cmd_id |= 0x2;
    if (fmt->z_offset >= 0)     state->cmd_id |= 0x1;
    state->fmt = fmt;

}

void rdpq_fan_add_vertex(const rdpq_trifmt_t *fmt, const float* vtx) {
    uint32_t cmd_id = RDPQ_CMD_FAN_ADD;
    if (fmt->shade_offset >= 0) cmd_id |= 0x4;
    if (fmt->tex_offset >= 0)   cmd_id |= 0x2;
    if (fmt->z_offset >= 0)     cmd_id |= 0x1;

    const int TRI_DATA_LEN = ROUND_UP((2+1+1+3)*4, 16);

    const float *v = vtx;

    // X,Y: s13.2
    int16_t x = floorf(v[fmt->pos_offset+0] * 4.0f);
    int16_t y = floorf(v[fmt->pos_offset+1] * 4.0f);
        
    int16_t z = 0;
    if (fmt->z_offset >= 0) {
        z = v[fmt->z_offset+0] * 0x7FFF;
    } 

    int32_t rgba = 0;
    if (fmt->shade_offset >= 0) {
        const float *v_shade = v;
        uint32_t r = v_shade[fmt->shade_offset+0] * 255.0;
        uint32_t g = v_shade[fmt->shade_offset+1] * 255.0;
        uint32_t b = v_shade[fmt->shade_offset+2] * 255.0;
        uint32_t a = v_shade[fmt->shade_offset+3] * 255.0;
        rgba = (r << 24) | (g << 16) | (b << 8) | a;
    }

    int16_t s=0, t=0;
    int32_t w=0, inv_w=0;
    if (fmt->tex_offset >= 0) {
        s     = v[fmt->tex_offset+0] * 32.0f;
        t     = v[fmt->tex_offset+1] * 32.0f;
        w     = float_to_s16_16(1.0f / v[fmt->tex_offset+2]);
        inv_w = float_to_s16_16(       v[fmt->tex_offset+2]);
    }

    rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE_DATA,
        TRI_DATA_LEN * 2, 
        (x << 16) | (y & 0xFFFF), 
        (z << 16), 
        rgba, 
        (s << 16) | (t & 0xFFFF), 
        w,
        inv_w);

}

void rdpq_fan_add_new_triangle(const float* v) {
    const int TRI_DATA_LEN = ROUND_UP((2+1+1+3)*4, 16);

    if (state->vtxCount == 0) {
        // Store the first vertex
        memcpy(state->v1, v, sizeof(state->v1));
        state->v1Added = true;
        state->vtxCount++;
        
    } else if (state->vtxCount == 1) {
        // Store first vertex as previous point
        memcpy(state->pv, state->v1, sizeof(state->pv));
        state->vtxCount++;
    } else if (state->vtxCount == 2) {

        // Draw the triangle with the center, previous vertex, and the new vertex using RDPQ_CMD_TRIANGLE_DATA
        const float *vtx[3] = { v, state->cv, state->pv};

        /*

        // Print vertices to debug the fan
        for (int i = 0; i < 3; i++) {
            const float *verts = vtx[i];
            int16_t x = floorf(verts[state->fmt->pos_offset + 0] * 4.0f);
            int16_t y = floorf(verts[state->fmt->pos_offset + 1] * 4.0f);
            debugf("Vertex %d: x = %d, y = %d (Original: x = %f, y = %f)\n", i+1, x, y, verts[state->fmt->pos_offset + 0], verts[state->fmt->pos_offset + 1]);
        }

        */

        for (int i = 0; i < 3; i++) {
            const float *verts = vtx[i];
            // X,Y: s13.2
            int16_t x = floorf(verts[state->fmt->pos_offset + 0] * 4.0f);
            int16_t y = floorf(verts[state->fmt->pos_offset + 1] * 4.0f);

            int16_t z = 0;
            if (state->fmt->z_offset >= 0) {
                z = verts[state->fmt->z_offset + 0] * 0x7FFF;
            }
            int32_t rgba = 0;
            if (state->fmt->shade_offset >= 0) {
                const float *v_shade = state->fmt->shade_flat ? vtx[0] : v;
                uint32_t r = v_shade[state->fmt->shade_offset + 0] * 255.0;
                uint32_t g = v_shade[state->fmt->shade_offset + 1] * 255.0;
                uint32_t b = v_shade[state->fmt->shade_offset + 2] * 255.0;
                uint32_t a = v_shade[state->fmt->shade_offset + 3] * 255.0;
                rgba = (r << 24) | (g << 16) | (b << 8) | a;
            }
            int16_t s = 0, t = 0;
            int32_t w = 0, inv_w = 0;
            if (state->fmt->tex_offset >= 0) {
                s = verts[state->fmt->tex_offset + 0] * 32.0f;
                t = verts[state->fmt->tex_offset + 1] * 32.0f;
                w = float_to_s16_16(1.0f / verts[state->fmt->tex_offset + 2]);
                inv_w = float_to_s16_16(verts[state->fmt->tex_offset + 2]);
            }
            rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE_DATA,
                TRI_DATA_LEN * i, 
                (x << 16) | (y & 0xFFFF), 
                (z << 16), 
                rgba, 
                (s << 16) | (t & 0xFFFF), 
                w,
                inv_w);
        }

        // Store current vertex and increment counter
        memcpy(state->pv, v, sizeof(state->pv));
        state->vtxCount++;

        // Render the command to draw the triangle
        rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE, 
            0xC000 | (state->cmd_id << 8) | 
            (state->fmt->tex_mipmaps ? (state->fmt->tex_mipmaps - 1) << 3 : 0) | 
            (state->fmt->tex_tile & 7));


    } else {

        // The idea is to hopefully just write the new vertex to RDPQ_TRI_DATA3
        rdpq_fan_add_vertex(state->fmt, v);
    }

}

// Function to close the fan and complete the shape
void rdpq_fan_end() {

    rdpq_fan_add_new_triangle(state->v1);

    free_uncached(state);
}


#endif // RDPQ_FAN_H