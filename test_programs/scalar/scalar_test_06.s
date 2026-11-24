# Scalar Test 06: Equivalent VL=8 element-wise array addition
main:
    li t0, 0x2000
    li t1, 0x2100
    li t2, 0x2200
    li t3, 8
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    addi s3, s0, 1
    sd s3, 0(s2)
    add s4, t1, s1
    li s5, 10
    sub s6, s5, s0
    sd s6, 0(s4)
    ld s7, 0(s2)
    ld s8, 0(s4)
    add s9, s7, s8
    add s10, t2, s1
    sd s9, 0(s10)
    addi s0, s0, 1
    blt s0, t3, loop
end:
    nop
