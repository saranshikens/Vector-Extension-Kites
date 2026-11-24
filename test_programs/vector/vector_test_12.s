# Vector Test 12: Maximum supported VL=64 add
main:
    li t0, 0x3300
    li t1, 0x3400
    li t2, 0x3500
    li t3, 64
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    sd s0, 0(s2)
    li s4, 1
    sd s4, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vadd.vv v3, v1, v2
    vse64.v v3, (t2)
end:
    nop
