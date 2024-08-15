#include <rsp_queue.inc>
#include <rdpq_constants.h>
#include <rdpq_macros.h>

    .func RDPQCmd_FanAddVertex
RDPQCmd_FanAddVertex:
    # Store the fourth vertex in memory
    sw a1, %lo(RDPQ_TRI_DATA3) + 0(a0)  # X/Y of Vertex 4
    sw a2, %lo(RDPQ_TRI_DATA3) + 4(a0)  # Z of Vertex 4
    sw a3, %lo(RDPQ_TRI_DATA3) + 8(a0)  # RGBA of Vertex 4

    lw t0, CMD_ADDR(16, 28)
    lw t1, CMD_ADDR(20, 28)
    lw t2, CMD_ADDR(24, 28)

    sw t0, %lo(RDPQ_TRI_DATA3) + 12(a0)  # S/T of Vertex 4
    sw t1, %lo(RDPQ_TRI_DATA3) + 16(a0)  # W of Vertex 4
    sw t2, %lo(RDPQ_TRI_DATA3) + 20(a0)  # INV_W of Vertex 4

    # Load the data for vertices 1, 3, and 4 from memory
    lw a1, %lo(RDPQ_TRI_DATA0) + 0(a0)   # X/Y of Vertex 1
    lw a2, %lo(RDPQ_TRI_DATA2) + 0(a0)   # X/Y of Vertex 3
    lw a3, %lo(RDPQ_TRI_DATA3) + 0(a0)   # X/Y of Vertex 4

    # Send this new triangle to RDP
    jal RDPQ_Triangle_Send_Async

    jr ra
    .endfunc