# Scalar Test 25: Equivalent adjacent-array processing and memory preservation
main:
    li t0, 0x5a00
    li t1, 0x5b00
    li t2, 0x5c00
    li t3, 4
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    add s4, t2, s1
    addi s5, s0, 1
    li s6, 3
    sd s5, 0(s2)
    sd s6, 0(s3)
    ld s7, 0(s2)
    ld s8, 0(s3)
    sub s9, s7, s8
    mul s10, s9, s8
    sd s10, 0(s4)
    addi s0, s0, 1
    blt s0, t3, loop
    li s11, 0x123456789abcdef0
    sd s11, 32(t2)
end:
    nop
