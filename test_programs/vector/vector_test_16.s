# Vector Test 16: Consecutive chained arithmetic dependencies
main:
    li t0, 0x3c00
    li t1, 0x3d00
    li t2, 0x3e00
    li t3, 4
    vsetvli x0, t3, 24
    li s0, 1
    li s1, 2
    li s2, 3
    li s3, 4
    sd s0, 0(t0)
    sd s1, 8(t0)
    sd s2, 16(t0)
    sd s3, 24(t0)
    li s0, 5
    sd s0, 0(t1)
    li s0, 6
    sd s0, 8(t1)
    li s0, 7
    sd s0, 16(t1)
    li s0, 8
    sd s0, 24(t1)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vadd.vv v3, v1, v2
    vsub.vv v4, v3, v1
    vmul.vv v5, v4, v2
    vse64.v v5, (t2)
end:
    nop
