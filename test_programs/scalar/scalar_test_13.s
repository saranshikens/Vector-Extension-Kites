# Scalar Test 13: Equivalent 64-lane result after AVL 100 cap
main:
    li t0, 0x3600
    li t1, 0x3700
    li t2, 64
    li s0, 0
loop:
    slli s1, s0, 3
    add s2, t0, s1
    add s3, t1, s1
    li s4, 3
    sd s4, 0(s2)
    ld s5, 0(s2)
    mul s6, s5, s5
    sd s6, 0(s3)
    addi s0, s0, 1
    blt s0, t2, loop
end:
    nop
