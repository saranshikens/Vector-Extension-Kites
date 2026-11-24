# Scalar Test 07: Equivalent alternating negative subtraction
main:
    li t0, 0x2300
    li t1, 0x2400
    li t2, 0x2500
    li t3, 8
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    li s3, 100
    add s3, s3, s0
    sd s3, 0(s2)
    add s4, t1, s1
    li s5, 0xffffffffffffffff
    sd s5, 0(s4)
    ld s6, 0(s2)
    sub s7, s6, s5
    add s8, t2, s1
    sd s7, 0(s8)
    addi s0, s0, 1
    blt s0, t3, loop
end:
    nop
