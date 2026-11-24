# Scalar Test 17: Equivalent subtract-multiply overflow semantics
main:
    li t0, 0x3f00
    li t1, 0x4000
    li t2, 0x4100
    li s5, 2
    li s6, 0xffffffffffffffff
    sd s5, 0(t1)
    sd s5, 8(t1)
    sd s5, 16(t1)
    sd s5, 24(t1)
    sd s6, 0(t0)
    sd s6, 8(t0)
    li s7, 0
    sd s7, 16(t0)
    sd s6, 24(t0)
    ld s7, 0(t0)
    ld s8, 0(t1)
    sub s9, s7, s8
    mul s10, s9, s8
    sd s10, 0(t2)
    ld s7, 8(t0)
    ld s8, 8(t1)
    sub s9, s7, s8
    mul s10, s9, s8
    sd s10, 8(t2)
    ld s7, 16(t0)
    ld s8, 16(t1)
    sub s9, s7, s8
    mul s10, s9, s8
    sd s10, 16(t2)
    ld s7, 24(t0)
    ld s8, 24(t1)
    sub s9, s7, s8
    mul s10, s9, s8
    sd s10, 24(t2)
end:
    nop
