# Scalar Test 05: Equivalent load-add-subtract-multiply chain
main:
    li t0, 0x1b00
    li t1, 0x1c00
    li t2, 0x1d00
    li s0, 3
    li s1, 7
    li s2, 12
    li s3, 20
    sd s0, 0(t0)
    sd s1, 8(t0)
    sd s2, 16(t0)
    sd s3, 24(t0)
    li s0, 1
    li s1, 2
    li s2, 3
    li s3, 4
    sd s0, 0(t1)
    sd s1, 8(t1)
    sd s2, 16(t1)
    sd s3, 24(t1)
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    ld s4, 0(s2)
    ld s5, 0(s3)
    add s6, s4, s5
    sub s7, s6, s5
    mul s8, s7, s5
    add s9, t2, s1
    sd s8, 0(s9)
    addi s0, s0, 1
    li s10, 4
    blt s0, s10, loop
end:
    nop
