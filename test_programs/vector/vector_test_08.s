# Vector Test 08: VL=8 multiplication by zero, one, and repeated elements
main:
    li t0, 0x2600
    li t1, 0x2700
    li t2, 0x2800
    li t3, 8
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    li s4, 7
    sd s4, 0(s2)
    li s4, 1
    blt s0, t3, set_b
set_b:
    li s5, 0
    li s6, 1
    blt s0, s6, store_b
    li s5, 1
store_b:
    sd s5, 0(s3)
    addi s0, s0, 1
    blt s0, t3, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    li s5, 0
    sd s5, 0(t1)
    vle64.v v2, (t1)
    vmul.vv v3, v1, v2
    vse64.v v3, (t2)
end:
    nop
