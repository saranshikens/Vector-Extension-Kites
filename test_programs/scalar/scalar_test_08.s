# Scalar Test 08: Equivalent multiplication by zero and one
main:
    li t0, 0x2600
    li t1, 0x2700
    li t2, 0x2800
    li t3, 8
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    li s4, 7
    sd s4, 0(s2)
    li s5, 1
    sd s5, 0(s3)
    ld s6, 0(s2)
    ld s7, 0(s3)
    mul s8, s6, s7
    add s9, t2, s1
    sd s8, 0(s9)
    addi s0, s0, 1
    blt s0, t3, loop
    li s11, 0
    sd s11, 0(t1)
end:
    nop
