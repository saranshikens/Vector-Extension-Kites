# Scalar Test 01: Equivalent VL=1 unsigned addition
main:
    li t0, 0x1000
    li t1, 0x1100
    li t2, 0
    li t3, 0xffffffffffffffff
    sd t2, 0(t0)
    sd t3, 0(t1)
    ld t4, 0(t0)
    ld t5, 0(t1)
    add t6, t4, t5
    sd t6, 0(t1)
end:
    nop
