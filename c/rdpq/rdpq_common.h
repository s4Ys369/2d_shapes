#ifndef RDPQ_COMMON_H
#define RDPQ_COMMON_H

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

typedef enum {
    TRI_DATA_NEXT,
    TRI_DATA_LAST,
    TRI_DATA_CENTER
} FAN_TRI_DATA_SLOT;

typedef enum {
    TRI_DATA_A,
    TRI_DATA_B,
    TRI_DATA_C
} STRIP_TRI_DATA_SLOT;

// This is the higher level call to add to TRI_DATA
void rdpq_add_tri_data(const rdpq_trifmt_t *fmt, const float* vtx, int triDataSlot) {

    if (!vtx) {
        debugf("rdpq_add_tri_data: Invalid arguments\n");
        return;
    }

    const int TRI_DATA_LEN = ROUND_UP((2+1+1+3)*4, 16);

    // Follow the normal steps for getting the vertex data
    uint32_t cmd_id = RDPQ_CMD_TRI;
    if (fmt->shade_offset >= 0) cmd_id |= 0x4;
    if (fmt->tex_offset >= 0)   cmd_id |= 0x2;
    if (fmt->z_offset >= 0)     cmd_id |= 0x1;


    int16_t x = floorf(vtx[fmt->pos_offset + 0] * 4.0f);
    int16_t y = floorf(vtx[fmt->pos_offset + 1] * 4.0f);

    int16_t z = 0;
    if (fmt->z_offset >= 0) {
        z = vtx[fmt->z_offset + 0] * 0x7FFF;
    }
    int32_t rgba = 0;
    if (fmt->shade_offset >= 0) {
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

#endif // RDPQ_COMMON_H