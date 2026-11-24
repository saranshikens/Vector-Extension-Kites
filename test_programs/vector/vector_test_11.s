# Vector Test 11: 10-element array processed as 4+4+2 chunks
main:
    li t0, 0x3000
    li t1, 0x3100
    li t2, 0x3200
    li t3, 10
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    addi s3, s0, 1
    sd s3, 0(s2)
    addi s0, s0, 1
    blt s0, t3, init
    li t3, 4
    vsetvli x0, t3, 24
    vle64.v v1, (t0)
    vadd.vv v2, v1, v1
    vse64.v v2, (t1)
    addi t0, t0, 32
    addi t1, t1, 32
    vle64.v v1, (t0)
    vadd.vv v2, v1, v1
    vse64.v v2, (t1)
    addi t0, t0, 32
    addi t1, t1, 32
    li t3, 2
    vsetvli x0, t3, 24
    vle64.v v1, (t0)
    vadd.vv v2, v1, v1
    vse64.v v2, (t1)
end:
    nop
