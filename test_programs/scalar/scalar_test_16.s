# Scalar Test 16: Equivalent consecutive chained arithmetic
main:
    li t0, 0x3c00
    li t1, 0x3d00
    li t2, 0x3e00
    li t3, 4
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    add s4, t2, s1
    addi s5, s0, 1
    addi s6, s0, 5
    sd s5, 0(s2)
    sd s6, 0(s3)
    ld s7, 0(s2)
    ld s8, 0(s3)
    add s9, s7, s8
    sub s10, s9, s7
    mul s11, s10, s8
    sd s11, 0(s4)
    addi s0, s0, 1
    blt s0, t3, loop
end:
    nop
