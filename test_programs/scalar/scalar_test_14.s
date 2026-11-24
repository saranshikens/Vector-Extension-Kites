# Scalar Test 14: Equivalent repeated scalar widths
main:
    li t0, 0x3800
    li t1, 0x3900
    li t2, 11
    sd t2, 0(t0)
    ld t3, 0(t0)
    sd t3, 0(t1)
    li t2, 12
    sd t2, 8(t0)
    ld t3, 8(t0)
    sd t3, 8(t1)
    li t2, 4
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    ld s4, 0(s2)
    mul s5, s4, s4
    sd s5, 0(s3)
    addi s0, s0, 1
    blt s0, t2, loop
end:
    nop
