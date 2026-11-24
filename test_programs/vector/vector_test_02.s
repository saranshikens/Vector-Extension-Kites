# Vector Test 02: VL=2 subtraction of identical and negative values
main:
    li t0, 0x1200
    li t1, 0x1300
    li t2, 0x1400
    li t3, 2
    vsetvli x0, t3, 24
    li t4, 10
    sd t4, 0(t0)
    li t4, 10
    sd t4, 8(t0)
    li t4, 0xffffffffffffffff
    sd t4, 0(t1)
    li t4, 5
    sd t4, 8(t1)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vsub.vv v3, v1, v2
    vse64.v v3, (t2)
end:
    nop
