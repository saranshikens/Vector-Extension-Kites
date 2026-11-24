# Scalar Test 02: Equivalent VL=2 subtraction
main:
    li t0, 0x1200
    li t1, 0x1300
    li t2, 0x1400
    li t3, 10
    li t4, 0xffffffffffffffff
    li t5, 5
    sd t3, 0(t0)
    sd t3, 8(t0)
    sd t4, 0(t1)
    sd t5, 8(t1)
    ld t3, 0(t0)
    ld t4, 0(t1)
    sub t6, t3, t4
    sd t6, 0(t2)
    ld t3, 8(t0)
    ld t4, 8(t1)
    sub t6, t3, t4
    sd t6, 8(t2)
end:
    nop
