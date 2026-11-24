# Scalar Test 18: Equivalent sum of squared differences
main:
    li t0, 0x4200
    li t1, 0x4300
    li t2, 0x4400
    li t3, 8
    li s0, 0
    li s1, 0
loop:
    slli s2, s1, 3
    add s3, t0, s2
    add s4, t1, s2
    addi s5, s1, 1
    li s6, 9
    sd s5, 0(s3)
    sd s6, 0(s4)
    ld s7, 0(s3)
    ld s8, 0(s4)
    sub s9, s7, s8
    mul s10, s9, s9
    add s11, t2, s2
    sd s10, 0(s11)
    add s0, s0, s10
    addi s1, s1, 1
    blt s1, t3, loop
    li s1, 0x4500
    sd s0, 0(s1)
end:
    nop
