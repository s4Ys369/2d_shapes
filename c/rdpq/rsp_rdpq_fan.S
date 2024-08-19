#include <rsp_queue.inc>
#include <rsp_rdpq.inc>

    .set noreorder
    .set at

    .data

    RSPQ_BeginOverlayHeader
        RSPQ_DefineCommand RDPQCmd_FanAddVertex,         16
    RSPQ_EndOverlayHeader

    RSPQ_BeginSavedState
        .align 4
RDPQ_TRI_DATA0:          .dcb.l 7
        .align 4
RDPQ_TRI_DATA1:          .dcb.l 7
    RSPQ_EndSavedState

    .text

    .func RDPQCmd_FanAddVertex
RDPQCmd_FanAddVertex:

    lh a1, %lo(RDPQ_TRI_DATA0) # load
    sh a1, 2(s3) # store

    # Now store the next vertex data into RDPQ_TRI_DATA0
    sw a1, %lo(RDPQ_TRI_DATA0) + 0(a0)   # Store X/Y of next vertex
    sw a2, %lo(RDPQ_TRI_DATA0) + 4(a0)   # Store Z of next vertex
    sw a3, %lo(RDPQ_TRI_DATA0) + 8(a0)   # Store RGBA of next vertex

    lw t0, CMD_ADDR(16, 28)   # Load S/T
    lw t1, CMD_ADDR(20, 28)   # Load W
    lw t2, CMD_ADDR(24, 28)   # Load INV_W

    sw t0, %lo(RDPQ_TRI_DATA0) + 12(a0)  # Store S/T of next vertex
    sw t1, %lo(RDPQ_TRI_DATA0) + 16(a0)  # Store W of next vertex
    sw t2, %lo(RDPQ_TRI_DATA0) + 20(a0)  # Store INV_W of next vertex

    li a1, %lo(RDPQ_TRI_DATA0)
    li a2, %lo(RDPQ_TRI_DATA1)
    jal RDPQ_Triangle_Send_Async
RDPQ_Triangle_Clip:
RDPQ_Triangle_Cull:
    j RSPQ_Loop
    .endfunc