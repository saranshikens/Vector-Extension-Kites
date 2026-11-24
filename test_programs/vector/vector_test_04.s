# Vector Test 04: VL=4 dot product with mixed signed representations
main:
    li t0, 0x1800
    li t1, 0x1900
    li t2, 4
    vsetvli x0, t2, 24
    li t3, 1
    sd t3, 0(t0)
    li t3, 0xffffffffffffffff
    sd t3, 8(t0)
    li t3, 3
    sd t3, 16(t0)
    li t3, 0xfffffffffffffffe
    sd t3, 24(t0)
    li t3, 5
    sd t3, 0(t1)
    li t3, 2
    sd t3, 8(t1)
    li t3, 4
    sd t3, 16(t1)
    li t3, 3
    sd t3, 24(t1)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vdot.vv s0, v1, v2
    li t3, 0x1a00
    sd s0, 0(t3)
end:
    nop
