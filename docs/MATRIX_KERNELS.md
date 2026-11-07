# Matrix kernel workloads

The assembly workloads in `examples/` exercise the current integer/vector ISA
on the five-stage pipelined VM. All elements are 64-bit integers, and each
program expects the following memory layout:

| Address | Contents |
| --- | --- |
| `0x2000` | Input matrix `A`, row-major |
| `0x2100` | Transposed input matrix `B`, row-major (matrix multiply only) |
| `0x2200` | Output matrix `C` or lower factor `L` |
| `0x2400` | Upper factor `U` (LU only) |

The 4x4 scalar and vector matrix multiplication programs compute identical
results. The vector version sets VL to four, loads each row of `A` once, and
uses `vdot.vv` against each contiguous row of `B-transpose`; this avoids both
the scalar inner-product loop and repeated loads of the same `A` row.
`test/matrix_kernels_test.cpp` checks the result and confirms that the vector
version retires fewer instructions and cycles on the hazard-detecting,
forwarding RV5 VM.

`cholesky_4x4.s` computes `A = L * transpose(L)` for symmetric positive
definite integer matrices whose Cholesky entries are integral. Its diagonal
square-root search returns the exact root for this input domain. `lu_4x4.s`
uses Doolittle factorization with a unit diagonal in `L`; it does not pivot,
so `U` must have nonzero pivots and each integer division must be exact.
These constraints keep the factorization results exact without adding
floating-point instructions that are outside this integer-kernel stage.

Run the workload checks with the normal test preset:

```sh
cmake --build --preset test-only
ctest --preset test-only -R MatrixKernelsTest
```
