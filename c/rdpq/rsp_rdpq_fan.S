#include <rsp_queue.inc>
#include <rsp_rdpq.inc>

#ifndef RDPQ_TRI_DATA0
#define RDPQ_TRI_DATA0 $4
#endif

#ifndef RDPQ_TRI_DATA1
#define RDPQ_TRI_DATA1 $5
#endif

    .set noreorder
    .set at

    .data

RSPQ_BeginOverlayHeader
    RSPQ_DefineCommand RDPQCmd_FanAddVertex,         28
RSPQ_EndOverlayHeader

RSPQ_BeginSavedState

RSPQ_EndSavedState

    .text

#ifndef RDPQ_Triangle_Cull
    .func RDPQ_Triangle_Cull
 RDPQ_Triangle_Cull:
    jr ra
    .endfunc
#endif

#ifndef RDPQ_Triangle_Clip
    .func RDPQ_Triangle_Clip
 RDPQ_Triangle_Clip:
    jr ra
    .endfunc
#endif

    .func RDPQCmd_FanAddVertex
RDPQCmd_FanAddVertex:

    # Copy the last vertex (RDPQ_TRI_DATA0) to the new slot (RDPQ_TRI_DATA1)
    lw t0, 0(RDPQ_TRI_DATA0)  # Copy X/Y of Last Vertex
    lw t1, 4(RDPQ_TRI_DATA0)  # Copy Z of Last Vertex
    lw t2, 8(RDPQ_TRI_DATA0)  # Copy RGBA of Last Vertex
    sw t0, 0(RDPQ_TRI_DATA1) 
    sw t1, 4(RDPQ_TRI_DATA1)
    sw t2, 8(RDPQ_TRI_DATA1)

    lw t0, 12(RDPQ_TRI_DATA0) # Copy S/T of Last Vertex
    lw t1, 16(RDPQ_TRI_DATA0) # Copy W of Last Vertex
    lw t2, 20(RDPQ_TRI_DATA0) # Copy INV_W of Last Vertex
    sw t0, 12(RDPQ_TRI_DATA1)
    sw t1, 16(RDPQ_TRI_DATA1)
    sw t2, 20(RDPQ_TRI_DATA1)

    # Store the next vertex in memory
    sw a1, 0(RDPQ_TRI_DATA0)  # X/Y of Next Vertex
    sw a2, 4(RDPQ_TRI_DATA0)  # Z of Next Vertex
    sw a3, 8(RDPQ_TRI_DATA0)  # RGBA of Next Vertex

    lw t0, CMD_ADDR(16, 28) # S/T Address
    lw t1, CMD_ADDR(20, 28) # W Address
    lw t2, CMD_ADDR(24, 28) # INV_W Address

    sw t0, 12(RDPQ_TRI_DATA0)  # S/T of Next Vertex
    sw t1, 16(RDPQ_TRI_DATA0)  # W of Next Vertex
    sw t2, 20(RDPQ_TRI_DATA0)  # INV_W of Next Vertex

    # Load the data for the current and previous vertices to draw the new triangle
    lw a1, 0(RDPQ_TRI_DATA0)   # X/Y of Next Vertex
    lw a2, 0(RDPQ_TRI_DATA1)   # X/Y of Last Vertex

    # Send this new triangle to RDP
    jal RDPQ_Triangle_Send_Async

    # Jump back to RA register
    jr ra
    .endfunc