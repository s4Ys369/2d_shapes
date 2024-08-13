#ifndef RDPQ_FAN_H
#define RDPQ_FAN_H

#include <libdragon.h>

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
    const float* cv; // Center vertex
    const float* v1; // First vertex of the fan after the center
    const float* pv; // Holds the previous vertex
    const rdpq_trifmt_t* fmt; // Triangle format
    uint32_t cmd_id; // RPDQ command ID
    bool v1Added; // Check to see any vertices have been added after rdpq_fan_begin
} rdpq_fan_t;

rdpq_fan_t* state;

rdpq_fan_t* rdpq_fan_init(){
    state = (rdpq_fan_t*)malloc_uncached(sizeof(rdpq_fan_t));
    state->cv = NULL;
    state->v1 = NULL;
    state->pv = NULL;
    state->fmt = NULL;
    state->cmd_id = 0;
    state->v1Added = false;
    return state;
}

// Function to initialize the fan drawing
void rdpq_fan_begin(const rdpq_trifmt_t *fmt, const float *cv) {
    state = rdpq_fan_init();
    state->cv = cv;
    state->v1Added = false;
    state->cmd_id = RDPQ_CMD_TRI;
    if (fmt->shade_offset >= 0) state->cmd_id |= 0x4;
    if (fmt->tex_offset >= 0)   state->cmd_id |= 0x2;
    if (fmt->z_offset >= 0)     state->cmd_id |= 0x1;
    state->fmt = fmt;

}

void rdpq_fan_add_vertex(const float* v) {
    if (!state->v1Added) {
        // Store the first vertex
        state->v1 = v;
        state->v1Added = true;
    } else {
        // Draw the triangle with the center, previous vertex, and the new vertex using RDPQ_CMD_TRIANGLE_DATA
        const float *vtx[3] = { state->cv, state->pv, v };

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
                0, 
                (x << 16) | (y & 0xFFFF), 
                (z << 16), 
                rgba, 
                (s << 16) | (t & 0xFFFF), 
                w,
                inv_w);
        }

        rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE, 
        0xC000 | (state->cmd_id << 8) | 
        (state->fmt->tex_mipmaps ? (state->fmt->tex_mipmaps - 1) << 3 : 0) | 
        (state->fmt->tex_tile & 7));

    }

    // Update the previous vertex to the current one
    state->pv = v;
}

// Function to close the fan and complete the shape
void rdpq_fan_end() {
    if (state->v1Added) {
        // Close the loop by adding the first vertex as the last one
        rdpq_fan_add_vertex(state->v1);
    }

    rspq_write(RDPQ_OVL_ID, RDPQ_CMD_TRIANGLE, 
        0xC000 | (state->cmd_id << 8) | 
        (state->fmt->tex_mipmaps ? (state->fmt->tex_mipmaps - 1) << 3 : 0) | 
        (state->fmt->tex_tile & 7));
    

    free_uncached(state);
}

#endif // RDPQ_FAN_H