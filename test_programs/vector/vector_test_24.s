# Vector Test 24: Alternating add-subtract-multiply chain
main:
    li t0, 0x5700
    li t1, 0x5800
    li t2, 0x5900
    li t3, 8
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    li s4, 0xffffffffffffffff
    sd s4, 0(s2)
    li s4, 2
    sd s4, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vadd.vv v3, v1, v2
    vsub.vv v4, v3, v1
    vmul.vv v5, v4, v2
    vse64.v v5, (t2)
end:
    nop
