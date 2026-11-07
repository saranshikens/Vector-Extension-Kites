# Vector-reduced Doolittle LU with unit diagonal L and no pivoting.
# A is row-major at 0x2000, L at 0x2200, and row-major U at 0x2400.
# Requires nonzero pivots and exact integer divisions.
# U-transpose is held at 0x2500 as scratch storage so U columns are contiguous
# vectors; each computed U element is also written to row-major output.
main:
    li t0, 0x2000
    li t1, 0x2200
    li t2, 0x2400
    li t3, 0x2500
    li t4, 4
    li s0, 0

row_loop:
    vsetvli x0, s0, 24
    slli s4, s0, 5
    add s5, t1, s4
    slli s4, s0, 5
    add s6, t3, s4
    li s1, 0
    vle64.v v1, (s5)
    vle64.v v2, (s6)

upper_column_loop:
    blt s1, s0, next_upper_column
    bge s1, t4, upper_done
    mul s4, s0, t4
    add s4, s4, s1
    slli s4, s4, 3
    add s5, t0, s4
    ld s7, 0(s5)
    slli s4, s1, 5
    add s5, t3, s4
    mul s4, s0, t4
    add s4, s4, s1
    slli s4, s4, 3
    add s10, t2, s4
    mul s4, s1, t4
    add s4, s4, s0
    slli s4, s4, 3
    add s11, t3, s4
    vle64.v v3, (s5)
    vdot.vv s6, v1, v3
    sub s7, s7, s6
    sd s7, 0(s11)
    sd s7, 0(s10)

next_upper_column:
    addi s1, s1, 1
    j upper_column_loop

upper_done:
    li s9, 1
    mul s4, s0, t4
    add s4, s4, s0
    slli s4, s4, 3
    add s5, t1, s4
    sd s9, 0(s5)
    addi s1, s0, 1

lower_row_loop:
    bge s1, t4, lower_done
    slli s4, s1, 5
    add s6, t1, s4

    mul s4, s1, t4
    add s4, s4, s0
    slli s4, s4, 3
    add s5, t0, s4
    ld s7, 0(s5)
    mul s4, s0, t4
    add s4, s4, s0
    slli s4, s4, 3
    add s5, t3, s4
    ld s9, 0(s5)
    mul s4, s1, t4
    add s4, s4, s0
    slli s4, s4, 3
    add s10, t1, s4
    vle64.v v3, (s6)
    vdot.vv s8, v3, v2
    sub s7, s7, s8
    div s7, s7, s9
    sd s7, 0(s10)
    addi s1, s1, 1
    j lower_row_loop

lower_done:
    addi s0, s0, 1
    blt s0, t4, row_loop

end:
    nop
