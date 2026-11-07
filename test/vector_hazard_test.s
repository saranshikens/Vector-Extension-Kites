# Vector hazard test: intentionally contains no NOP instructions.
# Expected initial memory at 0x100: 11, 22, 33, 44
# Expected v1 after vle64.v:       11, 22, 33, 44
# Expected v3 after vadd:           21, 42, 63, 84
# Expected v4 after vsub:           20, 40, 60, 80
# Expected memory at 0x200:         20, 40, 60, 80

    addi x10, x0, 256
    addi x12, x0, 512
    addi x11, x0, 11
    sd   x11, 0(x10)
    addi x11, x0, 22
    sd   x11, 8(x10)
    addi x11, x0, 33
    sd   x11, 16(x10)
    addi x11, x0, 44
    sd   x11, 24(x10)

    vle64.v v1, (x10)
    vadd.vv v3, v1, v2
    vsub.vv v4, v3, v1
    vse64.v v4, (x12)
