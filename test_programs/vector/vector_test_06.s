# Vector Test 06: VL=8 element-wise array addition
main:
    li t0, 0x2000
    li t1, 0x2100
    li t2, 0x2200
    li t3, 8
    vsetvli x0, t3, 24
    li s0, 0
init_a:
    slli s1, s0, 3
    add s2, t0, s1
    addi s3, s0, 1
    sd s3, 0(s2)
    addi s0, s0, 1
    blt s0, t3, init_a
    li s0, 0
init_b:
    slli s1, s0, 3
    add s2, t1, s1
    li s3, 10
    sub s4, s3, s0
    sd s4, 0(s2)
    addi s0, s0, 1
    blt s0, t3, init_b
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vadd.vv v3, v1, v2
    vse64.v v3, (t2)
end:
    nop
