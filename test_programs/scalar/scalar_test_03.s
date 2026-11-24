# Scalar Test 03: Equivalent VL=4 multiplication
main:
    li t0, 0x1500
    li t1, 0x1600
    li t2, 0x1700
    li t3, 0
    li t4, 1
    li t5, 7
    li t6, 0xffffffffffffffff
    sd t3, 0(t0)
    sd t4, 8(t0)
    sd t5, 16(t0)
    sd t6, 24(t0)
    li t3, 9
    li t4, 8
    li t5, 6
    li t6, 2
    sd t3, 0(t1)
    sd t4, 8(t1)
    sd t5, 16(t1)
    sd t6, 24(t1)
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    ld s4, 0(s2)
    ld s5, 0(s3)
    mul s6, s4, s5
    add s7, t2, s1
    sd s6, 0(s7)
    addi s0, s0, 1
    li s8, 4
    blt s0, s8, loop
end:
    nop
