# NH/NF version: software-schedule the scalar AVL producer.
#
# Expected:
#   x10 = 2
#   x5  = 2
#   VL  = 2
#   v3  = 11, 22

    addi     x10, x0, 2
    nop
    nop
    nop
    vsetvli  x5, x10, 24
    vsetvli  x0, x0, 24
    vadd.vv  v3, v1, v2