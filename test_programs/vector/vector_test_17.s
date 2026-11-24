# Vector Test 17: Subtract-multiply-store with max unsigned values
main:
    li t0, 0x3f00
    li t1, 0x4000
    li t2, 0x4100
    li t3, 4
    vsetvli x0, t3, 24
    li t4, 0xffffffffffffffff
    li t5, 0
    sd t4, 0(t0)
    sd t4, 8(t0)
    sd t5, 16(t0)
    sd t4, 24(t0)
    li t4, 2
    sd t4, 0(t1)
    sd t4, 8(t1)
    sd t4, 16(t1)
    sd t4, 24(t1)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vsub.vv v3, v1, v2
    vmul.vv v4, v3, v2
    vse64.v v4, (t2)
end:
    nop
