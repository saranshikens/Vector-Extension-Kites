# Custom KITES vdot.vv scalar reduction regression.
# Vector operands are initialized by the accompanying PowerShell runner.

# Full VL=4: [1,2,3,4] dot [5,6,7,8] = 70
li x10, 4
nop
nop
nop
vsetvli x5, x10, 24
vdot.vv x6, v1, v2
nop
nop
nop
addi x7, x6, 1        # immediate scalar consumer: expected x7=71

# VL=2: 1*5 + 2*6 = 17
li x10, 2
nop
nop
nop
vsetvli x5, x10, 24
vdot.vv x8, v1, v2

# VL=0: empty reduction = 0
li x10, 0
nop
nop
nop
vsetvli x5, x10, 24
vdot.vv x9, v1, v2

# Restore VL=4, exercise consecutive dot-products and x0 destination.
li x10, 4
nop
nop
nop
vsetvli x5, x10, 24
vdot.vv x12, v1, v2
vdot.vv x13, v2, v1
vdot.vv x0, v1, v2