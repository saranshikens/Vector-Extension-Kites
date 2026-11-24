# Vector Test 21: Processing 10 elements in 8 and 2 lane chunks
main:
    li t0, 0x4e00
    li t1, 0x4f00
    li t2, 0x5000
    li t3, 10
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    addi s3, s0, 20
    sd s3, 0(s2)
    addi s0, s0, 1
    blt s0, t3, init
    li t3, 8
    vsetvli x0, t3, 24
    vle64.v v1, (t0)
    vmul.vv v2, v1, v1
    vse64.v v2, (t1)
    addi t0, t0, 64
    addi t1, t1, 64
    li t3, 2
    vsetvli x0, t3, 24
    vle64.v v1, (t0)
    vmul.vv v2, v1, v1
    vse64.v v2, (t1)
end:
    nop
