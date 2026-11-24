# Vector Test 23: 64-bit addition overflow and underflow
main:
    li t0, 0x5400
    li t1, 0x5500
    li t2, 0x5600
    li t3, 4
    vsetvli x0, t3, 24
    li t4, 0xffffffffffffffff
    li t5, 1
    sd t4, 0(t0)
    sd t5, 0(t1)
    li t4, 0x8000000000000000
    li t5, 0x8000000000000000
    sd t4, 8(t0)
    sd t5, 8(t1)
    li t4, 0x7fffffffffffffff
    li t5, 1
    sd t4, 16(t0)
    sd t5, 16(t1)
    li t4, 0
    li t5, 0xffffffffffffffff
    sd t4, 24(t0)
    sd t5, 24(t1)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vadd.vv v3, v1, v2
    vse64.v v3, (t2)
end:
    nop
