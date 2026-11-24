# Vector Test 22: Small matrix row dot-product computation
main:
    li t0, 0x5100
    li t1, 0x5200
    li t2, 0x5300
    li t3, 4
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    addi s4, s0, 1
    li s5, 4
    sub s6, s5, s0
    sd s4, 0(s2)
    sd s6, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vdot.vv s0, v1, v2
    sd s0, 0(t2)
end:
    nop
