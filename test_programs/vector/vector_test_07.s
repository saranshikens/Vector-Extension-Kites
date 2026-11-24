# Vector Test 07: VL=8 subtraction with alternating negative values
main:
    li t0, 0x2300
    li t1, 0x2400
    li t2, 0x2500
    li t3, 8
    vsetvli x0, t3, 24
    li s0, 0
init:
    slli s1, s0, 3
    add s2, t0, s1
    li s3, 100
    add s3, s3, s0
    sd s3, 0(s2)
    add s2, t1, s1
    li s3, 0xffffffffffffffff
    sd s3, 0(s2)
    addi s0, s0, 1
    blt s0, t3, init
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vsub.vv v3, v1, v2
    vse64.v v3, (t2)
end:
    nop
