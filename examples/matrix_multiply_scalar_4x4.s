# Scalar 4x4 matrix multiply.
# A is row-major at 0x2000, B-transpose is row-major at 0x2100,
# and C is written row-major at 0x2200. Entries are 64-bit integers.
main:
    li t0, 0x2000
    li t1, 0x2100
    li t2, 0x2200
    li t3, 4
    li s0, 0

row_loop:
    li s1, 0

column_loop:
    li s2, 0
    li s3, 0

dot_loop:
    mul s4, s0, t3
    add s4, s4, s2
    slli s4, s4, 3
    add s5, t0, s4
    ld s6, 0(s5)

    mul s4, s1, t3
    add s4, s4, s2
    slli s4, s4, 3
    add s5, t1, s4
    ld s7, 0(s5)

    mul s8, s6, s7
    add s3, s3, s8
    addi s2, s2, 1
    blt s2, t3, dot_loop

    mul s4, s0, t3
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t2, s4
    sd s3, 0(s5)

    addi s1, s1, 1
    blt s1, t3, column_loop
    addi s0, s0, 1
    blt s0, t3, row_loop

end:
    nop
