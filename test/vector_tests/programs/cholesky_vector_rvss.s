
# ============================================================
# 8x8 VECTORIZED CHOLESKY DECOMPOSITION - RVSS
#
# A = L * L^T
# Column-major double-precision matrices.
#
# A base: 0x2000
# L base: 0x2200
#
# Vector operations:
#   vfdiv.vf
#   vfnmsac.vf
#   vle64.v
#   vse64.v
#
# Scalar operation:
#   fsqrt.d
#
# The input matrix A must be initialized in memory before run.
# ============================================================

main:
    li x10, 8192          # x10 = base address of A (0x2000)
    li x11, 8704          # x11 = base address of L (0x2200)
    li x17, 8             # Matrix dimension
    li x5, 0              # k = 0

# ------------------------------------------------------------
# Outer loop: pivot k
# ------------------------------------------------------------
pivot_loop:
    bge x5, x17, end

    # x6 = k * 64
    slli x6, x5, 6

    # x7 = k * 72 (diagonal offset)
    slli x7, x5, 3
    add x7, x7, x6

    # L[k,k] = sqrt(A[k,k])
    add x8, x10, x7
    fld f1, 0(x8)
    fsqrt.d f1, f1

    add x8, x11, x7
    fsd f1, 0(x8)

    # VL = 7-k
    li x14, 7
    sub x14, x14, x5

    # Last pivot has no rows below it.
    beq x14, x0, next_pivot

    # Configure vector length.
    vsetvli x0, x14, 24

# ------------------------------------------------------------
# Vector division:
# L[k+1:7,k] = A[k+1:7,k] / L[k,k]
# ------------------------------------------------------------

    # Address A[k*8 + (k+1)]
    addi x12, x5, 1
    slli x12, x12, 3
    add x12, x12, x6
    add x12, x12, x10

    vle64.v v1, (x12)

    # Divide every active lane by scalar L[k,k].
    vfdiv.vf v2, v1, f1

    # Address L[k*8 + (k+1)]
    addi x13, x5, 1
    slli x13, x13, 3
    add x13, x13, x6
    add x13, x13, x11

    vse64.v v2, (x13)

# ------------------------------------------------------------
# Load L[k+1:7,k] once for the trailing updates.
# ------------------------------------------------------------

    vle64.v v4, (x13)

# ------------------------------------------------------------
# For each column j > k:
#
# A[k+1:7,j] -= L[j,k] * L[k+1:7,k]
#
# Each update processes all remaining rows in parallel.
# ------------------------------------------------------------

    addi x9, x5, 1         # j = k+1

column_update_loop:
    bge x9, x17, next_pivot

    # Load scalar L[j,k]
    slli x15, x9, 3
    add x15, x15, x6
    add x16, x11, x15
    fld f2, 0(x16)

    # Address A[k+1:7,j]
    slli x12, x9, 6        # j*64
    addi x13, x5, 1
    slli x13, x13, 3       # (k+1)*8
    add x12, x12, x13
    add x12, x12, x10

    vle64.v v3, (x12)

    # v3 = v3 - f2 * v4
    vfnmsac.vf v3, f2, v4

    # Store updated column segment.
    vse64.v v3, (x12)

    addi x9, x9, 1
    jal x0, column_update_loop

next_pivot:
    addi x5, x5, 1
    jal x0, pivot_loop

end:
    nop
