# Scalar Test 11: Equivalent 10-element chunked array doubling
main:
    li t0, 0x3000
    li t1, 0x3100
    li t2, 10
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    addi s3, s0, 1
    sd s3, 0(s2)
    ld s4, 0(s2)
    add s5, s4, s4
    add s6, t1, s1
    sd s5, 0(s6)
    addi s0, s0, 1
    blt s0, t2, loop
end:
    nop
