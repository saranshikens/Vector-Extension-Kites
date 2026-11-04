# NH/NF vector dependency test
# Software-managed dependencies; no hardware forwarding or hazard detection.

    addi x10, x0, 256
    nop
    nop
    nop

    addi x12, x0, 512
    nop
    nop
    nop

    # Initialize memory: 11, 22, 33, 44

    addi x11, x0, 11
    nop
    nop
    nop
    sd x11, 0(x10)
    nop
    nop
    nop

    addi x11, x0, 22
    nop
    nop
    nop
    sd x11, 8(x10)
    nop
    nop
    nop

    addi x11, x0, 33
    nop
    nop
    nop
    sd x11, 16(x10)
    nop
    nop
    nop

    addi x11, x0, 44
    nop
    nop
    nop
    sd x11, 24(x10)
    nop
    nop
    nop

    # Load vector
    vle64.v v1, (x10)
    nop
    nop
    nop

    # v3 = v1 + v2
    vadd.vv v3, v1, v2
    nop
    nop
    nop

    # v4 = v3 - v1
    vsub.vv v4, v3, v1
    nop
    nop
    nop

    # Store vector
    vse64.v v4, (x12)
    nop
    nop
    nop