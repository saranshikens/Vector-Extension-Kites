# Vector Test 25: Multiple adjacent arrays with memory preservation
main:
    li t0, 0x5a00
    li t1, 0x5b00
    li t2, 0x5c00
    li t3, 4
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    addi s4, s0, 1
    li s5, 3
    sd s4, 0(s2)
    sd s5, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    li s6, 0x123456789abcdef0
    sd s6, 32(t2)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vsub.vv v3, v1, v2
    vmul.vv v4, v3, v2
    vse64.v v4, (t2)
end:
    nop
