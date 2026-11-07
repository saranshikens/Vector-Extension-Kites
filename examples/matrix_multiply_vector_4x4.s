# Vectorized 4x4 matrix multiply using the custom scalar-result vdot.vv.
# A is row-major at 0x2000, B-transpose is row-major at 0x2100,
# and C is written row-major at 0x2200. Entries are 64-bit integers.
main:
    li t0, 0x2000
    li t1, 0x2100
    li t2, 0x2200
    li t3, 4
    vsetvli x0, t3, 24
    li s0, 0

row_loop:
    slli s4, s0, 5
    add s5, t0, s4
    # Drain the scalar base-address dependency before vector memory access.
    nop
    nop
    nop
    vle64.v v1, (s5)
    li s1, 0

column_loop:
    slli s4, s1, 5
    add s5, t1, s4
    nop
    nop
    nop
    vle64.v v2, (s5)
    vdot.vv s3, v1, v2

    slli s4, s0, 5
    slli s5, s1, 3
    add s4, s4, s5
    add s4, t2, s4
    sd s3, 0(s4)

    addi s1, s1, 1
    blt s1, t3, column_loop
    addi s0, s0, 1
    blt s0, t3, row_loop

end:
    nop
