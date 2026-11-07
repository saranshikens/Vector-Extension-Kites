# Immediately consume the scalar vdot.vv result with no software NOPs.
# This is intended for forwarding-enabled and hazard-detection configurations.
li x10, 4
nop
nop
nop
vsetvli x5, x10, 24
vdot.vv x6, v1, v2
addi x7, x6, 1
