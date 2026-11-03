# Vector memory test for rv5-nh-nf (no hazard detection / no forwarding).
# Insert two explicit bubbles after each producer before a dependent consumer.

    addi x10, x0, 256       # source base = 0x100
    addi x12, x0, 512       # destination base = 0x200

    addi x11, x0, 11
    addi x0, x0, 0
    addi x0, x0, 0
    sd   x11, 0(x10)

    addi x11, x0, 22
    addi x0, x0, 0
    addi x0, x0, 0
    sd   x11, 8(x10)

    addi x11, x0, 33
    addi x0, x0, 0
    addi x0, x0, 0
    sd   x11, 16(x10)

    addi x11, x0, 44
    addi x0, x0, 0
    addi x0, x0, 0
    sd   x11, 24(x10)

    # Allow the final scalar store to complete before the vector load.
    addi x0, x0, 0
    addi x0, x0, 0
    addi x0, x0, 0

    vle64.v v1, (x10)

    # The vector load writes v1 in WB; wait before the dependent vector store.
    addi x0, x0, 0
    addi x0, x0, 0
    vse64.v v1, (x12)

    # Drain the vector store before stopping / inspecting memory.
    addi x0, x0, 0
    addi x0, x0, 0
    addi x0, x0, 0
