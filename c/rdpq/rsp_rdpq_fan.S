#include <rsp_queue.inc>
#include <rdpq_constants.h>
#include <rdpq_macros.h>

    .func RDPQCmd_FanAddVertex
RDPQCmd_FanAddVertex:

    # Copy the last vertex (RDPQ_TRI_DATA0) to the new slot (RDPQ_TRI_DATA1)

    lw t0, %lo(RDPQ_TRI_DATA0) + 0(a0)  # Copy X/Y of Last Vertex
    lw t1, %lo(RDPQ_TRI_DATA0) + 4(a0)  # Copy Z of Last Vertex
    lw t2, %lo(RDPQ_TRI_DATA0) + 8(a0)  # Copy RGBA of Last Vertex
    sw t0, %lo(RDPQ_TRI_DATA1) + 0(a0)
    sw t1, %lo(RDPQ_TRI_DATA1) + 4(a0)
    sw t2, %lo(RDPQ_TRI_DATA1) + 8(a0)

    lw t0, %lo(RDPQ_TRI_DATA0) + 12(a0) # Copy S/T of Last Vertex
    lw t1, %lo(RDPQ_TRI_DATA0) + 16(a0) # Copy W of Last Vertex
    lw t2, %lo(RDPQ_TRI_DATA0) + 20(a0) # Copy INV_W of Last Vertex
    sw t0, %lo(RDPQ_TRI_DATA1) + 12(a0)
    sw t1, %lo(RDPQ_TRI_DATA1) + 16(a0)
    sw t2, %lo(RDPQ_TRI_DATA1) + 20(a0)

    # Store the next vertex in memory
    sw a1, %lo(RDPQ_TRI_DATA0) + 0(a0)  # X/Y of Next Vertex
    sw a2, %lo(RDPQ_TRI_DATA0) + 4(a0)  # Z of Next Vertex
    sw a3, %lo(RDPQ_TRI_DATA0) + 8(a0)  # RGBA of Next Vertex

    lw t0, CMD_ADDR(16, 28) # S/T Address
    lw t1, CMD_ADDR(20, 28) # W Address
    lw t2, CMD_ADDR(24, 28) # INV_W Address

    sw t0, %lo(RDPQ_TRI_DATA0) + 12(a0)  # S/T of Next Vertex
    sw t1, %lo(RDPQ_TRI_DATA0) + 16(a0)  # W of Next Vertex
    sw t2, %lo(RDPQ_TRI_DATA0) + 20(a0)  # INV_W of Next Vertex

    # Load the data for the current and previous vertices to draw the new triangle
    lw a1, %lo(RDPQ_TRI_DATA0) + 0(a0)   # X/Y of Next Vertex
    lw a2, %lo(RDPQ_TRI_DATA1) + 0(a0)   # X/Y of Last Vertex

    # Send this new triangle to RDP
    jal RDPQ_Triangle_Send_Async

    # Jump back to RA register
    jr ra
    .endfunc