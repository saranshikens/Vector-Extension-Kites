# Vector Test 14: Repeated VL changes 1,2,4,8
main:
    li t0, 0x3800
    li t1, 0x3900
    li t2, 1
    vsetvli x0, t2, 24
    li t2, 11
    sd t2, 0(t0)
    vle64.v v1, (t0)
    vse64.v v1, (t1)
    li t2, 2
    vsetvli x0, t2, 24
    li t2, 12
    sd t2, 8(t0)
    vle64.v v2, (t0)
    vse64.v v2, (t1)
    li t2, 4
    vsetvli x0, t2, 24
    vle64.v v3, (t0)
    vmul.vv v4, v3, v3
    vse64.v v4, (t1)
end:
    nop
