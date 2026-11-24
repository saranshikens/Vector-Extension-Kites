# Kites RISC-V Simulator

Kites is a Qt/C++ RISC-V simulator with RVSS single-cycle execution, RV5S
five-stage pipeline configurations, assembly/CLI support, memory simulation,
floating-point support, and an implemented integer/vector subset.

This README describes setup and usage on Windows. Linux setup is possible, but
the supplied CMake presets use Windows Qt/MinGW paths. **NOTE THAT GUI WAS NOT IMPLEMENTED FOR THIS PROJECT, THEREFORE ONE MUST USE ONLY KIES CLI.**

## 1. Required tools

Install:

1. **Git**

   ```powershell
   git --version
   ```

2. **CMake 3.24 or newer**

   ```powershell
   cmake --version
   ```

3. **Ninja**

   ```powershell
   ninja --version
   ```

4. **Qt 6.9.3**

   The project requires Qt Core, Widgets, Network, and Concurrent.

5. **MinGW GCC/G++**

   The supplied presets use C++20 with Qt-bundled MinGW.

Optional:

- Visual Studio Code with C/C++ and CMake extensions.
- `gcovr` for coverage builds.
- GoogleTest is downloaded automatically during test configuration.

The supplied preset currently assumes:

```text
C:\Qt\6.9.3\mingw_64
C:\Qt\Tools\mingw1310_64
C:\Qt\Tools\Ninja
```


## 2. Configure Qt and compiler paths

The [CMakePresets.json](CMakePresets.json) `base` preset defines the Qt and
MinGW paths.

If Qt or MinGW is installed elsewhere, edit:

```json
"environment": {
  "PATH": "C:/your/location/mingw/bin;C:/your/location/Qt/bin;$penv{PATH}"
},
"cacheVariables": {
  "CMAKE_PREFIX_PATH": "C:/your/location/Qt/lib/cmake",
  "CMAKE_C_COMPILER": "C:/your/location/mingw/bin/gcc.exe",
  "CMAKE_CXX_COMPILER": "C:/your/location/mingw/bin/g++.exe"
}
```

The Qt CMake package must contain:

```text
Qt6Config.cmake
```

For the default preset, it is located at:

```text
C:\Qt\6.9.3\mingw_64\lib\cmake\Qt6\Qt6Config.cmake
```

No source-code changes are required for this standard setup.

## 3. Build the project

### Configure and build the CLI

```powershell
cmake --preset cli
cmake --build --preset cli
```

The binary is created at:

```text
build\cli\kites-cli.exe
```

Build only the CLI target explicitly:

```powershell
cmake --build build/cli --target kites-cli
```


## 4. Run assembly files with the Kites CLI

The CLI usage is:

```text
kites-cli <assembly-file> [--vm <name>] [--mem-dump <hex_addr> <row_count>]...
```

Add Qt/MinGW runtime directories to `PATH` if Windows cannot find Qt DLLs:

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.9.3\mingw_64\bin;$env:PATH"
```

### Run an RVSS single-cycle program

`rvss` is the default processor:

```powershell
.\build\cli\kites-cli.exe .\examples\matrix_multiply_scalar_4x4.s --vm rvss
```

It can be omitted:

```powershell
.\build\cli\kites-cli.exe .\examples\matrix_multiply_scalar_4x4.s
```

### Run an RV5S pipeline configuration

Available processor names:

| CLI name | Pipeline mode |
|---|---|
| `rvss` | Single-cycle |
| `rv5-nh-nf` | No hazard detection, no forwarding |
| `rv5-h-nf` | Hazard detection, no forwarding |
| `rv5-nh-f` | No hazard detection, forwarding |
| `rv5-h-f` | Hazard detection, forwarding |

Examples:

```powershell
.\build\cli\kites-cli.exe .\test_programs\vector\vector_test_04.s --vm rv5-nh-nf
.\build\cli\kites-cli.exe .\test_programs\vector\vector_test_04.s --vm rv5-h-nf
.\build\cli\kites-cli.exe .\test_programs\vector\vector_test_04.s --vm rv5-nh-f
.\build\cli\kites-cli.exe .\test_programs\vector\vector_test_04.s --vm rv5-h-f
```

### Dump memory

The address is hexadecimal and the row count is decimal:

```powershell
.\build\cli\kites-cli.exe .\test_programs\vector\vector_test_01.s `
  --vm rvss `
  --mem-dump 1000 4 `
  --mem-dump 1100 4
```

After a run, state is written under `vm_state/`:

- `registers_dump.json`
- `disassembly.txt`
- `errors_dump.json`
- `memory_dump.json` when `--mem-dump` is used

Exit codes:

- `0`: assembly and execution completed.
- `1`: invocation or assembly failed.
- `2`: runtime failure.
- `3`: memory dump failure.

See [docs/CLI.md](docs/CLI.md) for the complete CLI documentation.

## 5. Configure and build GoogleTests

Configure tests:

```powershell
cmake --preset test
```

Build the normal test executable:

```powershell
cmake --build build/test --target kites_test
```

Build the organized vector test executable:

```powershell
cmake --build build/test --target kites_vector_test
```

Add Qt runtime directories before direct executable launches:

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.9.3\mingw_64\bin;$env:PATH"
```

## 6. Run GoogleTests with CTest

Run all configured tests:

```powershell
ctest --preset test-only
```

Or:

```powershell
ctest --test-dir build/test --output-on-failure
```

Run only the vector suites:

```powershell
ctest --test-dir build/test --output-on-failure `
  -R "SingleCycleVector|PipelinedVector|ScalarVector"
```

The vector suites are:

- `IntegerVectorEdgeAndRandomCases`
- `PipelinedVectorEdgeAndRandomCases`
- `OneHundredScalarVectorKernels`

Run the vector executable directly:

```powershell
.\build\test\test\vector_tests\kites_vector_test.exe
```

List available tests:

```powershell
.\build\test\test\vector_tests\kites_vector_test.exe --gtest_list_tests
```

Run one suite:

```powershell
.\build\test\test\vector_tests\kites_vector_test.exe `
  --gtest_filter="IntegerVectorEdgeAndRandomCases/*"
```

Run one pipelined mode and correctness case family:

```powershell
.\build\test\test\vector_tests\kites_vector_test.exe `
  --gtest_filter="PipelinedVectorEdgeAndRandomCases/*/NH_F*"
```

Run the 100 scalar/vector comparison cases:

```powershell
.\build\test\test\vector_tests\kites_vector_test.exe `
  --gtest_filter="OneHundredScalarVectorKernels/*"
```

Run the ordinary project tests directly:

```powershell
.\build\test\kites_test.exe
```

## 7. Run the generated vector/scalar test programs

Generated programs are stored in:

- [test_programs/vector](test_programs/vector)
- [test_programs/scalar](test_programs/scalar)
- [test_programs/expected_outputs](test_programs/expected_outputs/expected_outputs.txt)

Example vector/scalar pair:

```powershell
.\build\cli\kites-cli.exe .\test_programs\vector\vector_test_04.s --vm rvss
.\build\cli\kites-cli.exe .\test_programs\scalar\scalar_test_04.s --vm rvss
```

Compare the outputs using `registers_dump.json`, memory dumps, and the
independent expected results.

## 8. Optional coverage build

Coverage requires GCC/MinGW and `gcovr`.

```powershell
cmake --preset coverage
cmake --build --preset test-coverage
ctest --preset test-coverage
cmake --build build/coverage --target coverage
```

The HTML report is written to:

```text
build\coverage\coverage\index.html
```

See [docs/TESTING.md](docs/TESTING.md) for test and coverage details.