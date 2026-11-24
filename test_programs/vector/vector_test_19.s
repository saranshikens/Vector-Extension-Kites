# Vector Test 19: Squared Euclidean distance over two 8-element arrays
main:
    li t0, 0x4600
    li t1, 0x4700
    li t2, 0x4800
    li t3, 8
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    li s4, 20
    sub s5, s4, s0
    sd s5, 0(s2)
    li s4, 3
    add s5, s4, s0
    sd s5, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vsub.vv v3, v1, v2
    vmul.vv v4, v3, v3
    vse64.v v4, (t2)
    vdot.vv s0, v3, v3
    li s1, 0x4900
    sd s0, 0(s1)
end:
    nop
