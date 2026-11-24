# Vector Test 03: VL=4 multiplication, zero and one operands
main:
    li t0, 0x1500
    li t1, 0x1600
    li t2, 0x1700
    li t3, 4
    vsetvli x0, t3, 24
    li t4, 0
    sd t4, 0(t0)
    li t4, 1
    sd t4, 8(t0)
    li t4, 7
    sd t4, 16(t0)
    li t4, 0xffffffffffffffff
    sd t4, 24(t0)
    li t4, 9
    sd t4, 0(t1)
    li t4, 8
    sd t4, 8(t1)
    li t4, 6
    sd t4, 16(t1)
    li t4, 2
    sd t4, 24(t1)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vmul.vv v3, v1, v2
    vse64.v v3, (t2)
end:
    nop
