# Vector-reduced integer Cholesky factorization, A = L * transpose(L).
# A is row-major at 0x2000 and L is written row-major at 0x2200.
# Requires an SPD input with integral Cholesky entries and exact diagonal roots.
main:
    li t0, 0x2000
    li t1, 0x2200
    li t2, 4
    li s0, 0

row_loop:
    li s1, 0

column_loop:
    mul s4, s0, t2
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t0, s4
    ld s3, 0(s5)

    mul s4, s0, t2
    add s4, s4, s1
    slli s4, s4, 3
    add s10, t1, s4
    vsetvli x0, s1, 24
    slli s4, s0, 5
    add s5, t1, s4
    slli s4, s1, 5
    add s6, t1, s4
    mul s4, s1, t2
    add s4, s4, s1
    slli s4, s4, 3
    add s7, t1, s4
    ld s9, 0(s7)
    vle64.v v1, (s5)
    vle64.v v2, (s6)
    vdot.vv s6, v1, v2
    sub s3, s3, s6

    bne s0, s1, off_diagonal
    li s8, 0

sqrt_loop:
    mul s9, s8, s8
    bge s9, s3, store_factor
    addi s8, s8, 1
    j sqrt_loop

off_diagonal:
    div s8, s3, s9

store_factor:
    sd s8, 0(s10)

    addi s1, s1, 1
    bge s0, s1, column_loop
    addi s0, s0, 1
    blt s0, t2, row_loop

end:
    nop
