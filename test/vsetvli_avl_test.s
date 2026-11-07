# KITES vsetvli AVL boundary test.
#
# The regression script sets x10 before running this program.
#
# Expected:
#   AVL=0   -> x5=0
#   AVL=100 -> x5=64

    vsetvli  x5, x10, 24