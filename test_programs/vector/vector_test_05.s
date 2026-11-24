# Vector Test 05: Load-add-subtract-multiply chain
main:
    li t0, 0x1b00
    li t1, 0x1c00
    li t2, 0x1d00
    li t3, 4
    vsetvli x0, t3, 24
    li t4, 3
    sd t4, 0(t0)
    li t4, 7
    sd t4, 8(t0)
    li t4, 12
    sd t4, 16(t0)
    li t4, 20
    sd t4, 24(t0)
    li t4, 1
    sd t4, 0(t1)
    li t4, 2
    sd t4, 8(t1)
    li t4, 3
    sd t4, 16(t1)
    li t4, 4
    sd t4, 24(t1)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vadd.vv v3, v1, v2
    vsub.vv v4, v3, v2
    vmul.vv v5, v4, v2
    vse64.v v5, (t2)
end:
    nop
