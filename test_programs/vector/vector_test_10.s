# Vector Test 10: Loading into two regions and storing into separate outputs
main:
    li t0, 0x2c00
    li t1, 0x2d00
    li t2, 0x2e00
    li t3, 0x2f00
    li t4, 4
    vsetvli x0, t4, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    addi s4, s0, 11
    sd s4, 0(s2)
    li s5, 5
    sd s5, 0(s3)
    addi s0, s0, 1
    blt s0, t4, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vadd.vv v3, v1, v2
    vse64.v v3, (t2)
    vmul.vv v4, v3, v2
    vse64.v v4, (t3)
end:
    nop
