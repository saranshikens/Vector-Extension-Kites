
.text
.globl main

main:
    # ---------------------------------------------------------
    # 4x4 Cholesky decomposition: A = L * L^T 2+14+87 = 103
    #
    # A is stored column-major at 0x2000.
    # L is stored column-major at 0x2200.
    #
    # Input:
    #   16  8  8  4
    #    8 29  9 12
    #    8  9 14  7
    #    4 12  7 10
    #
    # Expected L:
    #    4  0  0  0
    #    2  5  0  0
    #    2  1  3  0
    #    1  2  1  2
    # ---------------------------------------------------------

    li x10, 0x2000             # A base
    li x11, 0x2200             # L base

    # Clear the output matrix L.
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
    # L[0,0] = sqrt(A[0,0]) = 4
    # =========================================================

    fld f0, 0(x10)
    fsqrt.d f0, f0
    fsd f0, 0(x11)

    # L[1,0] = A[1,0] / L[0,0] = 2
    fld f1, 8(x10)
    fdiv.d f1, f1, f0
    fsd f1, 8(x11)

    # L[2,0] = A[2,0] / L[0,0] = 2
    fld f2, 16(x10)
    fdiv.d f2, f2, f0
    fsd f2, 16(x11)

    # L[3,0] = A[3,0] / L[0,0] = 1
    fld f3, 24(x10)
    fdiv.d f3, f3, f0
    fsd f3, 24(x11)

    # Update column 1:
    # A[1,1] -= L[1,0] * L[1,0]
    fld f4, 40(x10)
    fmul.d f5, f1, f1
    fsub.d f4, f4, f5
    fsd f4, 40(x10)

    # A[2,1] -= L[2,0] * L[1,0]
    fld f4, 48(x10)
    fmul.d f5, f2, f1
    fsub.d f4, f4, f5
    fsd f4, 48(x10)

    # A[3,1] -= L[3,0] * L[1,0]
    fld f4, 56(x10)
    fmul.d f5, f3, f1
    fsub.d f4, f4, f5
    fsd f4, 56(x10)

    # Update column 2:
    # A[1,2] -= L[1,0] * L[2,0]
    fld f4, 72(x10)
    fmul.d f5, f1, f2
    fsub.d f4, f4, f5
    fsd f4, 72(x10)

    # A[2,2] -= L[2,0] * L[2,0]
    fld f4, 80(x10)
    fmul.d f5, f2, f2
    fsub.d f4, f4, f5
    fsd f4, 80(x10)

    # A[3,2] -= L[3,0] * L[2,0]
    fld f4, 88(x10)
    fmul.d f5, f3, f2
    fsub.d f4, f4, f5
    fsd f4, 88(x10)

    # Update column 3:
    # A[1,3] -= L[1,0] * L[3,0]
    fld f4, 104(x10)
    fmul.d f5, f1, f3
    fsub.d f4, f4, f5
    fsd f4, 104(x10)

    # A[2,3] -= L[2,0] * L[3,0]
    fld f4, 112(x10)
    fmul.d f5, f2, f3
    fsub.d f4, f4, f5
    fsd f4, 112(x10)

    # A[3,3] -= L[3,0] * L[3,0]
    fld f4, 120(x10)
    fmul.d f5, f3, f3
    fsub.d f4, f4, f5
    fsd f4, 120(x10)

    # =========================================================
    # PIVOT 1
    # L[1,1] = sqrt(A[1,1]) = 5
    # =========================================================

    fld f0, 40(x10)
    fsqrt.d f0, f0
    fsd f0, 40(x11)

    # L[2,1] = A[2,1] / L[1,1] = 1
    fld f1, 48(x10)
    fdiv.d f1, f1, f0
    fsd f1, 48(x11)

    # L[3,1] = A[3,1] / L[1,1] = 2
    fld f2, 56(x10)
    fdiv.d f2, f2, f0
    fsd f2, 56(x11)

    # Update column 2.
    # A[2,2] -= L[2,1] * L[2,1]
    fld f4, 80(x10)
    fmul.d f5, f1, f1
    fsub.d f4, f4, f5
    fsd f4, 80(x10)

    # A[3,2] -= L[3,1] * L[2,1]
    fld f4, 88(x10)
    fmul.d f5, f2, f1
    fsub.d f4, f4, f5
    fsd f4, 88(x10)

    # Update column 3.
    # A[2,3] -= L[2,1] * L[3,1]
    fld f4, 112(x10)
    fmul.d f5, f1, f2
    fsub.d f4, f4, f5
    fsd f4, 112(x10)

    # A[3,3] -= L[3,1] * L[3,1]
    fld f4, 120(x10)
    fmul.d f5, f2, f2
    fsub.d f4, f4, f5
    fsd f4, 120(x10)

    # =========================================================
    # PIVOT 2
    # L[2,2] = sqrt(A[2,2]) = 3
    # =========================================================

    fld f0, 80(x10)
    fsqrt.d f0, f0
    fsd f0, 80(x11)

    # L[3,2] = A[3,2] / L[2,2] = 1
    fld f1, 88(x10)
    fdiv.d f1, f1, f0
    fsd f1, 88(x11)

    # Update final diagonal:
    # A[3,3] -= L[3,2] * L[3,2]
    fld f4, 120(x10)
    fmul.d f5, f1, f1
    fsub.d f4, f4, f5
    fsd f4, 120(x10)

    # =========================================================
    # PIVOT 3
    # L[3,3] = sqrt(A[3,3]) = 2
    # =========================================================

    fld f0, 120(x10)
    fsqrt.d f0, f0
    fsd f0, 120(x11)

end:
    nop
