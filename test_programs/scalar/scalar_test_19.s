# Scalar Test 19: Equivalent squared Euclidean distance
main:
    li t0, 0x4600
    li t1, 0x4700
    li t2, 0x4800
    li t3, 8
    li s0, 0
    li s1, 0
loop:
    slli s2, s1, 3
    add s3, t0, s2
    add s4, t1, s2
    li s5, 20
    sub s6, s5, s1
    li s5, 3
    add s7, s5, s1
    sd s6, 0(s3)
    sd s7, 0(s4)
    sub s8, s6, s7
    mul s9, s8, s8
    add s10, t2, s2
    sd s9, 0(s10)
    add s0, s0, s9
    addi s1, s1, 1
    blt s1, t3, loop
    li s11, 0x4900
    sd s0, 0(s11)
end:
    nop
