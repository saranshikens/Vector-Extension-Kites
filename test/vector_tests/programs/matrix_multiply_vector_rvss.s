
# Optimized 4x4 matrix multiplication on RVSS.
#
# A:           0x2000 (row-major)
# B-transpose: 0x2100 (row-major)
# C:           0x2200 (row-major)
#
# Optimization:
#   1. Load all four rows of A into v1-v4.
#   2. Load all four rows of B-transpose into v5-v8.
#   3. Set VL = 4 once.
#   4. Compute all 16 dot products.
#   5. Store results directly in row-major order.
#
# Each matrix element is a 64-bit integer.

main:
    # Configure vector length.
    li t2, 4
    vsetvli x0, t2, 24

    # ------------------------------------------------
    # Load all rows of matrix A.
    # ------------------------------------------------

    li t0, 0x2000

    vle64.v v1, (t0)
    addi t0, t0, 32

    vle64.v v2, (t0)
    addi t0, t0, 32

    vle64.v v3, (t0)
    addi t0, t0, 32

    vle64.v v4, (t0)

    # ------------------------------------------------
    # Load all rows of transposed matrix B.
    # ------------------------------------------------

    li t1, 0x2100

    vle64.v v5, (t1)
    addi t1, t1, 32

    vle64.v v6, (t1)
    addi t1, t1, 32

    vle64.v v7, (t1)
    addi t1, t1, 32

    vle64.v v8, (t1)

    # ------------------------------------------------
    # Matrix multiplication.
    #
    # C[i][j] = dot(A[i], B-transpose[j])
    #
    # Each dot product uses four vector lanes.
    # ------------------------------------------------

    li t3, 0x2200

    # Row 0
    vdot.vv s0, v1, v5
    sd s0, 0(t3)

    vdot.vv s0, v1, v6
    sd s0, 8(t3)

    vdot.vv s0, v1, v7
    sd s0, 16(t3)

    vdot.vv s0, v1, v8
    sd s0, 24(t3)

    # Row 1
    vdot.vv s0, v2, v5
    sd s0, 32(t3)

    vdot.vv s0, v2, v6
    sd s0, 40(t3)

    vdot.vv s0, v2, v7
    sd s0, 48(t3)

    vdot.vv s0, v2, v8
    sd s0, 56(t3)

    # Row 2
    vdot.vv s0, v3, v5
    sd s0, 64(t3)

    vdot.vv s0, v3, v6
    sd s0, 72(t3)

    vdot.vv s0, v3, v7
    sd s0, 80(t3)

    vdot.vv s0, v3, v8
    sd s0, 88(t3)

    # Row 3
    vdot.vv s0, v4, v5
    sd s0, 96(t3)

    vdot.vv s0, v4, v6
    sd s0, 104(t3)

    vdot.vv s0, v4, v7
    sd s0, 112(t3)

    vdot.vv s0, v4, v8
    sd s0, 120(t3)

end:
    nop
