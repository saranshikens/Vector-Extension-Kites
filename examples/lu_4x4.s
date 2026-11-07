# Doolittle LU factorization with unit diagonal L and no pivoting.
# A is row-major at 0x2000, L at 0x2200, and U at 0x2400.
# Entries are 64-bit integers; all divisions must be exact and U's diagonal
# pivots must be nonzero.
main:
    li t0, 0x2000
    li t1, 0x2200
    li t2, 0x2400
    li t3, 4
    li s0, 0

row_loop:
    add s1, s0, x0
upper_column_loop:
    bge s1, t3, upper_done
    mul s4, s0, t3
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t0, s4
    ld s3, 0(s5)
    li s2, 0

upper_product_loop:
    bge s2, s0, upper_product_done
    mul s4, s0, t3
    add s4, s4, s2
    slli s4, s4, 3
    add s5, t1, s4
    ld s6, 0(s5)
    mul s4, s2, t3
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t2, s4
    ld s7, 0(s5)
    mul s6, s6, s7
    sub s3, s3, s6
    addi s2, s2, 1
    j upper_product_loop

upper_product_done:
    mul s4, s0, t3
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t2, s4
    sd s3, 0(s5)
    addi s1, s1, 1
    j upper_column_loop

upper_done:
    li s8, 1
    mul s4, s0, t3
    add s4, s4, s0
    slli s4, s4, 3
    add s5, t1, s4
    sd s8, 0(s5)
    addi s1, s0, 1

lower_row_loop:
    bge s1, t3, lower_done
    mul s4, s1, t3
    add s4, s4, s0
    slli s4, s4, 3
    add s5, t0, s4
    ld s3, 0(s5)
    li s2, 0

lower_product_loop:
    bge s2, s0, lower_product_done
    mul s4, s1, t3
    add s4, s4, s2
    slli s4, s4, 3
    add s5, t1, s4
    ld s6, 0(s5)
    mul s4, s2, t3
    add s4, s4, s0
    slli s4, s4, 3
    add s5, t2, s4
    ld s7, 0(s5)
    mul s6, s6, s7
    sub s3, s3, s6
    addi s2, s2, 1
    j lower_product_loop

lower_product_done:
    mul s4, s0, t3
    add s4, s4, s0
    slli s4, s4, 3
    add s5, t2, s4
    ld s6, 0(s5)
    div s8, s3, s6
    mul s4, s1, t3
    add s4, s4, s0
    slli s4, s4, 3
    add s5, t1, s4
    sd s8, 0(s5)
    addi s1, s1, 1
    j lower_row_loop

lower_done:
    addi s0, s0, 1
    blt s0, t3, row_loop

end:
    nop
