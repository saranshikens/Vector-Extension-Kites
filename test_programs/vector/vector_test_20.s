# Vector Test 20: Multiply-and-accumulate
main:
    li t0, 0x4a00
    li t1, 0x4b00
    li t2, 0x4c00
    li t3, 8
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    addi s4, s0, 2
    li s5, 3
    sd s4, 0(s2)
    sd s5, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    li s6, 0
    li s7, 8
zero_acc:
    slli s8, s6, 3
    add s9, t2, s8
    li s10, 0
    sd s10, 0(s9)
    addi s6, s6, 1
    blt s6, s7, zero_acc
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vmul.vv v3, v1, v2
    vle64.v v4, (t2)
    vadd.vv v5, v3, v4
    vse64.v v5, (t2)
    vdot.vv s0, v5, v2
    li s1, 0x4d00
    sd s0, 0(s1)
end:
    nop
