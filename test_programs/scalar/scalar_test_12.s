# Scalar Test 12: Equivalent maximum-VL 64-element add
main:
    li t0, 0x3300
    li t1, 0x3400
    li t2, 0x3500
    li t3, 64
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    add s4, t2, s1
    sd s0, 0(s2)
    li s5, 1
    sd s5, 0(s3)
    ld s6, 0(s2)
    ld s7, 0(s3)
    add s8, s6, s7
    sd s8, 0(s4)
    addi s0, s0, 1
    blt s0, t3, loop
end:
    nop
