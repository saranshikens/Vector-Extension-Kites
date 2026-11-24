# Scalar Test 04: Equivalent four-lane dot product
main:
    li t0, 0x1800
    li t1, 0x1900
    li t2, 0x1a00
    li t3, 1
    li t4, 0xffffffffffffffff
    li t5, 3
    li t6, 0xfffffffffffffffe
    sd t3, 0(t0)
    sd t4, 8(t0)
    sd t5, 16(t0)
    sd t6, 24(t0)
    li t3, 5
    li t4, 2
    li t5, 4
    li t6, 3
    sd t3, 0(t1)
    sd t4, 8(t1)
    sd t5, 16(t1)
    sd t6, 24(t1)
    li s0, 0
    li s1, 0
    li s2, 4
loop:
    slli s3, s1, 3
    add s4, t0, s3
    add s5, t1, s3
    ld s6, 0(s4)
    ld s7, 0(s5)
    mul s8, s6, s7
    add s0, s0, s8
    addi s1, s1, 1
    blt s1, s2, loop
    sd s0, 0(t2)
end:
    nop
