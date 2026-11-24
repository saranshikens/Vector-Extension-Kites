# Vector Test Pair Summary

The 25 vector/scalar pairs cover basic instruction verification, arithmetic edge cases,
VL changes, memory regions, chained instructions, and application-style computations.

All programs use VTYPE 24 (E64/M1) for implementation compatibility. Floating-point
vector instructions are intentionally excluded.

| No. | Test name | Instructions | Edge case | Complexity |
|---|---|---|---|---|
| 01 | VL=1 unsigned wrap add | vsetvli,vle64,vadd,vse64 | Zero and UINT64_MAX | Simple |
| 02 | VL=2 signed representation subtract | vsetvli,vle64,vsub,vse64 | Identical elements, negative representation | Simple |
| 03 | VL=4 multiplication | vsetvli,vle64,vmul,vse64 | Zero, one, UINT64_MAX | Simple |
| 04 | VL=4 dot product | vsetvli,vle64,vdot | Mixed positive/negative representations | Simple |
| 05 | Chained arithmetic | vle64,vadd,vsub,vmul,vse64 | Consecutive RAW dependencies | Simple |
| 06 | 8-element addition | vsetvli,vle64,vadd,vse64 | Small array | Intermediate |
| 07 | 8-element subtraction | vsetvli,vle64,vsub,vse64 | Negative operands | Intermediate |
| 08 | Zero-and-one multiplication | vsetvli,vle64,vmul,vse64 | Zero and repeated elements | Intermediate |
| 09 | 8-element dot product | vsetvli,vle64,vdot | Large operands | Intermediate |
| 10 | Separate memory regions | vle64,vadd,vmul,vse64 | Different input/output regions | Intermediate |
| 11 | 10-element chunks | vsetvli,vle64,vadd,vse64 | Array length not divisible by 4 | Intermediate |
| 12 | Maximum VL | vsetvli,vle64,vadd,vse64 | VL=64 | Intermediate |
| 13 | AVL greater than VLMAX | vsetvli,vle64,vmul,vse64 | AVL capped to 64 | Intermediate |
| 14 | Repeated VL changes | vsetvli,vle64,vmul,vse64 | VL 1/2/4 | Intermediate |
| 15 | Repeated source load | vle64,vmul,vse64 | Same source loaded twice | Intermediate |
| 16 | Chained vector arithmetic | vadd,vsub,vmul,vse64 | Consecutive vector dependencies | Intermediate |
| 17 | Maximum unsigned arithmetic | vle64,vsub,vmul,vse64 | Overflow/wraparound | Intermediate |
| 18 | Sum of squared differences | vsub,vmul,vdot | Alternating differences | Complex |
| 19 | Squared Euclidean distance | vsub,vmul,vdot | 8-element application | Complex |
| 20 | Multiply-and-accumulate | vmul,vadd,vdot,vse64 | Intermediate accumulation | Complex |
| 21 | Chunked squaring | vsetvli,vle64,vmul,vse64 | 8+2 chunk processing | Complex |
| 22 | Matrix row dot product | vsetvli,vle64,vdot | Small matrix row | Complex |
| 23 | Overflow and underflow | vsetvli,vle64,vadd,vse64 | Signed representations | Complex |
| 24 | Alternating arithmetic chain | vadd,vsub,vmul,vse64 | UINT64_MAX operands | Complex |
| 25 | Adjacent arrays | vle64,vsub,vmul,vse64 | Memory preservation | Complex |

## Vector-to-scalar mapping

Each `vector_test_NN.s` has a corresponding `scalar_test_NN.s` at the same address
layout and with the same input values and output addresses.

## Pipeline dependencies

- Load-use: vector loads followed by arithmetic.
- RAW: arithmetic result used by the next arithmetic instruction or store.
- Store dependency: vector arithmetic result used by `vse64.v`.
- Configuration dependency: `vsetvli` followed by vector memory/arithmetic.
- Scalar dependency: `vdot.vv` followed by `sd` or scalar arithmetic.

## Processor compatibility

| Processor | Vector support | Scalar support | Validation result |
|---|---|---|---|
| RVSS | Supported | Supported | 25 vector + 25 scalar passed assembly/run |
| RV5-NH-NF | Supported | Supported | 25 vector + 25 scalar passed assembly/run |
| RV5-H-NF | Supported | Supported | 25 vector + 25 scalar passed assembly/run |
| RV5-NH-F | Supported | Supported | 25 vector + 25 scalar passed assembly/run |
| RV5-H-F | Supported | Supported | 25 vector + 25 scalar passed assembly/run |

The compatibility matrix reflects assembly/run validation. Correctness for pipeline
outputs requires memory/register verification using each program's independent expected
result in `expected_outputs.txt`.

## Instruction coverage matrix

| Instruction | Tests |
|---|---|
| vsetvli | 01,02,03,04,05,06,07,08,09,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25 |
| vle64.v | 01-25 |
| vse64.v | 01-25 |
| vadd.vv | 01,05,06,10,16,18,20,23,24 |
| vsub.vv | 02,05,07,16,17,18,19,24,25 |
| vmul.vv | 03,05,08,10,13,15,16,17,18,19,20,21,23,24,25 |
| vdot.vv | 04,09,18,19,20,22 |
