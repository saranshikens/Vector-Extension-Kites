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
| `0x2500` | Transposed `U` scratch storage (vector LU only) |

The 4x4 scalar and vector matrix multiplication programs compute identical
results. The vector version sets VL to four, loads each row of `A` once, and
uses `vdot.vv` against each contiguous row of `B-transpose`; this avoids both
the scalar inner-product loop and repeated loads of the same `A` row.
`test/matrix_kernels_test.cpp` checks the result and confirms that the vector
version retires fewer instructions and cycles on the hazard-detecting,
forwarding RV5 VM.

`cholesky_4x4.s` computes `A = L * transpose(L)` for symmetric positive
definite integer matrices whose Cholesky entries are integral. For each
`L[i][j]`, it sets VL to `j`, loads the already-computed prefixes of rows `i`
and `j`, and uses `vdot.vv` for their product sum. The diagonal square-root
search returns exact roots for this input domain. The prior scalar version is
preserved as `cholesky_scalar_4x4.s`.

`lu_4x4.s` uses Doolittle factorization with a unit diagonal in `L`; it does
not pivot, so `U` must have nonzero pivots and each integer division must be
exact. It sets VL to the current factorization row, uses `vdot.vv` to reduce
the product sums, and keeps a transposed copy of computed `U` in scratch
memory at `0x2500` so each needed column is contiguous for vector loads.
Computed values are also written to the public row-major `U` output. The prior
scalar version is preserved as `lu_scalar_4x4.s`. These constraints keep the
factorization results exact without requiring floating-point instructions.

On the tested 4x4 workload, the vector-reduced Cholesky kernel retires 362
instructions in 461 simulated cycles, compared with 422 instructions and 577
cycles for its scalar baseline. The vector-reduced LU kernel retires 492
instructions in 607 cycles, compared with 542 instructions and 707 cycles for
its scalar baseline. The same tests verify each factor matrix before asserting
the instruction and cycle improvements. These are simulator measurements,
not wall-clock timings; the gains are specific to this 4x4 input and should
not be assumed to predict larger matrices or real-hardware performance.

Run the workload checks with the normal test preset:

```sh
cmake --build --preset test-only
ctest --preset test-only -R MatrixKernelsTest
```
