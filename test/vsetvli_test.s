# KITES vsetvli test for forwarding / hardware hazard handling.
#
# The scalar producer writes AVL=2 immediately before vsetvli.
#
# Expected:
#   x10 = 2
#   x5  = 2
#   VL  = 2
#   v3  = 11, 22

    addi     x10, x0, 2
    vsetvli  x5, x10, 24
    vsetvli  x0, x0, 24
    vadd.vv  v3, v1, v2