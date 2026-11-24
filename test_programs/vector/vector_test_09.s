# Vector Test 09: VL=8 dot product with repeated and large elements
main:
    li t0, 0x2900
    li t1, 0x2a00
    li t2, 0x2b00
    li t3, 8
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    li s4, 0x7fffffffffffffff
    sd s4, 0(s2)
    li s4, 2
    sd s4, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vdot.vv s0, v1, v2
    sd s0, 0(t2)
end:
    nop
