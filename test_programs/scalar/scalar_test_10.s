# Scalar Test 10: Equivalent separate-region loads and stores
main:
    li t0, 0x2c00
    li t1, 0x2d00
    li t2, 0x2e00
    li t3, 0x2f00
    li t4, 4
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    addi s4, s0, 11
    li s5, 5
    sd s4, 0(s2)
    sd s5, 0(s3)
    ld s6, 0(s2)
    ld s7, 0(s3)
    add s8, s6, s7
    mul s9, s8, s7
    add s10, t2, s1
    add s11, t3, s1
    sd s8, 0(s10)
    sd s9, 0(s11)
    addi s0, s0, 1
    blt s0, t4, loop
end:
    nop
