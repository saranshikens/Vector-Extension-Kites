# vdot.vv undo/redo: reduction writes x6; vector state and VL are unchanged.
li x10, 2
nop
nop
nop
vsetvli x5, x10, 24
vdot.vv x6, v1, v2