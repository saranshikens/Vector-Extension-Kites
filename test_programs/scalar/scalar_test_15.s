# Scalar Test 15: Equivalent repeated loads
main:
    li t0, 0x3a00
    li t1, 0x3b00
    li t2, 4
    li t3, 6
    li t4, 7
    li t5, 8
    li t6, 9
    sd t3, 0(t0)
    sd t4, 8(t0)
    sd t5, 16(t0)
    sd t6, 24(t0)
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    ld s4, 0(s2)
    ld s5, 0(s2)
    mul s6, s4, s5
    sd s6, 0(s3)
    addi s0, s0, 1
    blt s0, t2, loop
end:
    nop
