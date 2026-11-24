# Scalar Test 09: Equivalent VL=8 dot product
main:
    li t0, 0x2900
    li t1, 0x2a00
    li t2, 0x2b00
    li t3, 8
    li s0, 0
    li s1, 0
loop:
    slli s2, s1, 3
    add s3, t0, s2
    add s4, t1, s2
    li s5, 0x7fffffffffffffff
    li s6, 2
    sd s5, 0(s3)
    sd s6, 0(s4)
    ld s7, 0(s3)
    ld s8, 0(s4)
    mul s9, s7, s8
    add s0, s0, s9
    addi s1, s1, 1
    blt s1, t3, loop
    sd s0, 0(t2)
end:
    nop
