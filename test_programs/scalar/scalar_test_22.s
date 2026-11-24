# Scalar Test 22: Equivalent 4-element matrix row dot product
main:
    li t0, 0x5100
    li t1, 0x5200
    li t2, 0x5300
    li t3, 4
    li s0, 0
    li s1, 0
loop:
    slli s2, s1, 3
    add s3, t0, s2
    add s4, t1, s2
    addi s5, s1, 1
    li s6, 4
    sub s7, s6, s1
    sd s5, 0(s3)
    sd s7, 0(s4)
    ld s8, 0(s3)
    ld s9, 0(s4)
    mul s10, s8, s9
    add s0, s0, s10
    addi s1, s1, 1
    blt s1, t3, loop
    sd s0, 0(t2)
end:
    nop
