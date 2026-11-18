
# ============================================================
# 8x8 SCALAR CHOLESKY DECOMPOSITION - RVSS
#
# A = L * L^T
# Column-major double-precision matrices.
#
# A base: 0x2000
# L base: 0x2200
#
# Uses:
#   fsqrt.d
#   fdiv.d
#   fmul.d
#   fsub.d
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

    # x6 = k * 64 (column offset in bytes)
    slli x6, x5, 6

    # x7 = k * 72 = (k*8 + k)*8 (diagonal offset)
    slli x7, x5, 3
    add x7, x7, x6

    # L[k,k] = sqrt(A[k,k])
    add x8, x10, x7
    fld f1, 0(x8)
    fsqrt.d f1, f1

    add x8, x11, x7
    fsd f1, 0(x8)

# ------------------------------------------------------------
# Compute L[i,k] = A[i,k] / L[k,k], i = k+1 ... 7
# ------------------------------------------------------------
    addi x9, x5, 1

column_division_loop:
    bge x9, x17, update_setup

    # Address A[k*8 + i]
    slli x12, x9, 3
    add x12, x12, x6
    add x13, x10, x12

    fld f2, 0(x13)
    fdiv.d f2, f2, f1

    # Store L[i,k]
    add x13, x11, x12
    fsd f2, 0(x13)

    addi x9, x9, 1
    jal x0, column_division_loop

# ------------------------------------------------------------
# Trailing matrix update:
# A[i,j] = A[i,j] - L[i,k] * L[j,k]
# ------------------------------------------------------------
update_setup:
    addi x9, x5, 1         # j = k+1

column_update_loop:
    bge x9, x17, next_pivot

    # Load L[j,k] into f2
    slli x12, x9, 3
    add x12, x12, x6
    add x13, x11, x12
    fld f2, 0(x13)

    # x14 = j * 64 (column j offset)
    slli x14, x9, 6

    # i = k+1
    addi x18, x5, 1

row_update_loop:
    bge x18, x17, next_column

    # Address A[i,j] = A + j*64 + i*8
    slli x15, x18, 3
    add x15, x15, x14
    add x16, x10, x15

    fld f3, 0(x16)

    # Address L[i,k] = L + k*64 + i*8
    slli x15, x18, 3
    add x15, x15, x6
    add x16, x11, x15

    fld f4, 0(x16)

    # A[i,j] -= L[j,k] * L[i,k]
    fmul.d f5, f2, f4
    fsub.d f3, f3, f5

    # Store updated A[i,j]
    slli x15, x18, 3
    add x15, x15, x14
    add x16, x10, x15
    fsd f3, 0(x16)

    addi x18, x18, 1
    jal x0, row_update_loop

next_column:
    addi x9, x9, 1
    jal x0, column_update_loop

next_pivot:
    addi x5, x5, 1
    jal x0, pivot_loop

end:
    nop
