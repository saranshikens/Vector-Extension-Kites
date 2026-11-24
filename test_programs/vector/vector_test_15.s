# Vector Test 15: Repeated loads from the same source region
main:
    li t0, 0x3a00
    li t1, 0x3b00
    li t2, 4
    vsetvli x0, t2, 24
    li t3, 6
    sd t3, 0(t0)
    li t3, 7
    sd t3, 8(t0)
    li t3, 8
    sd t3, 16(t0)
    li t3, 9
    sd t3, 24(t0)
    vle64.v v1, (t0)
    vle64.v v2, (t0)
    vmul.vv v3, v1, v2
    vse64.v v3, (t1)
end:
    nop
