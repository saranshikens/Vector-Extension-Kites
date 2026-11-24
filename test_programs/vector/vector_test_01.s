# Vector Test 01: VL=1 addition, including zero and unsigned wrap
main:
    li t0, 0x1000
    li t1, 0x1100
    li t2, 0
    li t3, 1
    li t4, 24
    vsetvli x0, t4, 24
    li t4, 1
    vsetvli x0, t4, 24
    li t5, 0
    sd t5, 0(t0)
    li t5, 0xffffffffffffffff
    sd t5, 0(t1)
    vle64.v v1, (t0)
    vle64.v v2, (t1)
    vadd.vv v3, v1, v2
    vse64.v v3, (t1)
end:
    nop
