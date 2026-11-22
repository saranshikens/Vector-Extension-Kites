
.text
.globl main

main:
    # ---------------------------------------------------------
    # 4x4 vectorized Cholesky decomposition 2+16+64 = 82    #
    # A base = 0x2000
    # L base = 0x2200
    # Column-major double-precision matrices.
    #
    # Uses:
    #   vsetvli
    #   vle64.v
    #   vse64.v
    #   vfdiv.vf
    #   vfnmsac.vf
    # ---------------------------------------------------------

    li x10, 0x2000             # A base
    li x11, 0x2200             # L base

    # Clear L.
    sd x0, 0(x11)
    sd x0, 8(x11)
    sd x0, 16(x11)
    sd x0, 24(x11)
    sd x0, 32(x11)
    sd x0, 40(x11)
    sd x0, 48(x11)
    sd x0, 56(x11)
    sd x0, 64(x11)
    sd x0, 72(x11)
    sd x0, 80(x11)
    sd x0, 88(x11)
    sd x0, 96(x11)
    sd x0, 104(x11)
    sd x0, 112(x11)
    sd x0, 120(x11)

    # =========================================================
    # PIVOT 0
    # =========================================================

    fld f0, 0(x10)
    fsqrt.d f0, f0
    fsd f0, 0(x11)

    # VL = 3: process rows 1, 2, 3.
    li x6, 3
    vsetvli x5, x6, 24

    # L[1:3,0] = A[1:3,0] / L[0,0]
    addi x12, x10, 8
    vle64.v v1, (x12)
    vfdiv.vf v2, v1, f0

    addi x13, x11, 8
    vse64.v v2, (x13)

    # Update column 1:
    # A[1:3,1] -= L[1:3,0] * L[1,0]
    fld f1, 8(x11)
    addi x12, x10, 40
    vle64.v v3, (x12)
    vfnmsac.vf v3, f1, v2
    vse64.v v3, (x12)

    # Update column 2:
    # A[1:3,2] -= L[1:3,0] * L[2,0]
    fld f1, 16(x11)
    addi x12, x10, 72
    vle64.v v3, (x12)
    vfnmsac.vf v3, f1, v2
    vse64.v v3, (x12)

    # Update column 3:
    # A[1:3,3] -= L[1:3,0] * L[3,0]
    fld f1, 24(x11)
    addi x12, x10, 104
    vle64.v v3, (x12)
    vfnmsac.vf v3, f1, v2
    vse64.v v3, (x12)

    # =========================================================
    # PIVOT 1
    # =========================================================

    fld f0, 40(x10)
    fsqrt.d f0, f0
    fsd f0, 40(x11)

    # VL = 2: process rows 2 and 3.
    li x6, 2
    vsetvli x5, x6, 24

    # L[2:3,1] = A[2:3,1] / L[1,1]
    addi x12, x10, 48
    vle64.v v1, (x12)
    vfdiv.vf v2, v1, f0

    addi x13, x11, 48
    vse64.v v2, (x13)

    # Update column 2.
    fld f1, 48(x11)
    addi x12, x10, 80
    vle64.v v3, (x12)
    vfnmsac.vf v3, f1, v2
    vse64.v v3, (x12)

    # Update column 3.
    fld f1, 56(x11)
    addi x12, x10, 112
    vle64.v v3, (x12)
    vfnmsac.vf v3, f1, v2
    vse64.v v3, (x12)

    # =========================================================
    # PIVOT 2
    # =========================================================

    fld f0, 80(x10)
    fsqrt.d f0, f0
    fsd f0, 80(x11)

    # VL = 1: process row 3.
    li x6, 1
    vsetvli x5, x6, 24

    # L[3,2] = A[3,2] / L[2,2]
    addi x12, x10, 88
    vle64.v v1, (x12)
    vfdiv.vf v2, v1, f0

    addi x13, x11, 88
    vse64.v v2, (x13)

    # Update final diagonal.
    fld f1, 88(x11)
    addi x12, x10, 120
    vle64.v v3, (x12)
    vfnmsac.vf v3, f1, v2
    vse64.v v3, (x12)

    # =========================================================
    # PIVOT 3
    # =========================================================

    fld f0, 120(x10)
    fsqrt.d f0, f0
    fsd f0, 120(x11)

end:
    nop
