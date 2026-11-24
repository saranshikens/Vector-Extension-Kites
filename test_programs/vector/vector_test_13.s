# Vector Test 13: Requested AVL 100 is capped at VLMAX
main:
    li t0, 0x3600
    li t1, 0x3700
    li t2, 100
    vsetvli x0, t2, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    li s3, 3
    sd s3, 0(s2)
    addi s0, s0, 1
    li s4, 64
    blt s0, s4, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vmul.vv v3, v1, v1
    vse64.v v3, (t1)
end:
    nop
