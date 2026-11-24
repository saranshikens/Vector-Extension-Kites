# Scalar Test 23: Equivalent modulo-2^64 overflow and underflow
main:
    li t0, 0x5400
    li t1, 0x5500
    li t2, 0x5600
    li t3, 0xffffffffffffffff
    li t4, 1
    sd t3, 0(t0)
    sd t4, 0(t1)
    li t3, 0x8000000000000000
    sd t3, 8(t0)
    sd t3, 8(t1)
    li t3, 0x7fffffffffffffff
    li t4, 1
    sd t3, 16(t0)
    sd t4, 16(t1)
    li t3, 0
    li t4, 0xffffffffffffffff
    sd t3, 24(t0)
    sd t4, 24(t1)
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    add s4, t2, s1
    ld s5, 0(s2)
    ld s6, 0(s3)
    add s7, s5, s6
    sd s7, 0(s4)
    addi s0, s0, 1
    li s8, 4
    blt s0, s8, loop
end:
    nop
