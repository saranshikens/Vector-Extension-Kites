# Scalar Test 24: Equivalent alternating add-subtract-multiply chain
main:
    li t0, 0x5700
    li t1, 0x5800
    li t2, 0x5900
    li t3, 8
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    add s4, t2, s1
    li s5, 0xffffffffffffffff
    li s6, 2
    sd s5, 0(s2)
    sd s6, 0(s3)
    ld s7, 0(s2)
    ld s8, 0(s3)
    add s9, s7, s8
    sub s10, s9, s7
    mul s11, s10, s8
    sd s11, 0(s4)
    addi s0, s0, 1
    blt s0, t3, loop
end:
    nop
