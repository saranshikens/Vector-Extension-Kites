# Execution Instructions

Build the CLI:

```powershell
cmake --preset cli
cmake --build --preset cli --target kites-cli
```

Run one program for RVSS:

```powershell
.\build\cli\kites-cli.exe test_programs\vector\vector_test_01.s --vm rvss
```

Run the corresponding scalar program under RV5S H/F:

```powershell
.\build\cli\kites-cli.exe test_programs\scalar\scalar_test_01.s --vm rv5-h-f
```

Supported processor names:

- `rvss`
- `rv5-nh-nf`
- `rv5-h-nf`
- `rv5-nh-f`
- `rv5-h-f`

For each pair, run the vector and scalar program with the same processor configuration.
Use `--vm` to select the processor. Record cycles and register/memory contents after
successful completion.

The CLI output includes instruction words and dump file locations. Use the REPL
commands `info`, `regs`, `pmem`, and `dmem` for detailed results.
