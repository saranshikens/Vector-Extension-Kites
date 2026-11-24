# Vector Test 18: Sum of squared differences
main:
    li t0, 0x4200
    li t1, 0x4300
    li t2, 0x4400
    li t3, 8
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    addi s4, s0, 1
    li s5, 9
    sd s4, 0(s2)
    sd s5, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vsub.vv v3, v1, v2
    vmul.vv v4, v3, v3
    vse64.v v4, (t2)
    vdot.vv s0, v3, v3
    li s1, 0x4500
    sd s0, 0(s1)
end:
    nop
