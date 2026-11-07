# Integer Cholesky factorization, A = L * transpose(L).
# A is row-major at 0x2000 and L is written row-major at 0x2200.
# This exact-integer version requires an SPD matrix whose Cholesky entries
# and diagonal square roots are integers (the test workload meets this rule).
main:
    li t0, 0x2000
    li t1, 0x2200
    li t2, 4
    li s0, 0

row_loop:
    li s1, 0

column_loop:
    mul s4, s0, t2
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t0, s4
    ld s3, 0(s5)

    li s2, 0
product_loop:
    bge s2, s1, product_done
    mul s4, s0, t2
    add s4, s4, s2
    slli s4, s4, 3
    add s5, t1, s4
    ld s6, 0(s5)

    mul s4, s1, t2
    add s4, s4, s2
    slli s4, s4, 3
    add s5, t1, s4
    ld s7, 0(s5)
    mul s6, s6, s7
    sub s3, s3, s6
    addi s2, s2, 1
    j product_loop

product_done:
    bne s0, s1, off_diagonal
    li s8, 0

sqrt_loop:
    mul s9, s8, s8
    bge s9, s3, store_factor
    addi s8, s8, 1
    j sqrt_loop

off_diagonal:
    mul s4, s1, t2
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t1, s4
    ld s9, 0(s5)
    div s8, s3, s9

store_factor:
    mul s4, s0, t2
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t1, s4
    sd s8, 0(s5)

    addi s1, s1, 1
    bge s0, s1, column_loop
    addi s0, s0, 1
    blt s0, t2, row_loop

end:
    nop
