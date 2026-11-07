# KITES vsetvli VLMAX test.
#
# rs1=x0 and rd!=x0 selects VLMAX.
#
# Expected:
#   x5 = 64

    vsetvli  x5, x0, 24