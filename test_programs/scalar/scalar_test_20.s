# Scalar Test 20: Equivalent multiply-and-accumulate
main:
    li t0, 0x4a00
    li t1, 0x4b00
    li t2, 0x4c00
    li s0, 0
    li s1, 2
    li s2, 3
    li s3, 0
loop0:
    sd s1, 0(t0)
    sd s2, 0(t1)
    sd s3, 0(t2)
    ld s4, 0(t0)
    ld s5, 0(t1)
    ld s6, 0(t2)
    mul s7, s4, s5
    add s8, s7, s6
    sd s8, 0(t2)
    mul s9, s8, s5
    add s0, s0, s9
    addi t0, t0, 8
    addi t1, t1, 8
    addi t2, t2, 8
    addi s1, s1, 1
    addi s3, s3, 1
    li s10, 10
    blt s1, s10, loop0
    li s11, 19712
    sd s0, 0(s11)
end:
    nop
