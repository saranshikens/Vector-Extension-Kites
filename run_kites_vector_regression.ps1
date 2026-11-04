
# KITES Vector Pipeline Regression Runner
# Run from the project root:
#   .\run_kites_vector_regression.ps1
#
# Build first:
#   cmake --build build --config Debug

$ErrorActionPreference = "Continue"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$exe = Join-Path $projectRoot "build\debug\kites-cli.exe"
$reportPath = Join-Path $projectRoot "vector_pipeline_regression_report.txt"

if (-not (Test-Path $exe)) {
    Write-Error "CLI executable not found: $exe. Build the project first."
    exit 1
}

$tests = @(
    @{
        Vm = "rv5-nh-nf"
        Program = "examples\vector_hazard_test_nh_nf.s"
        Label = "NH/NF (software NOPs)"
    },
    @{
        Vm = "rv5-nh-f"
        Program = "examples\vector_hazard_test_nh_f.s"
        Label = "NH/F (forwarding + load-use NOP)"
    },
    @{
        Vm = "rv5-h-nf"
        Program = "examples\vector_hazard_test.s"
        Label = "H/NF (hardware hazard detection)"
    },
    @{
        Vm = "rv5-h-f"
        Program = "examples\vector_hazard_test.s"
        Label = "H/F (hazard detection + forwarding)"
    }
)

$report = [System.Collections.Generic.List[string]]::new()
$summary = [System.Collections.Generic.List[object]]::new()

foreach ($test in $tests) {

    Write-Host ""
    Write-Host "Running $($test.Vm) ..." -ForegroundColor Cyan

    $programPath = Join-Path $projectRoot $test.Program

    if (-not (Test-Path $programPath)) {
        Write-Warning "Missing test program: $programPath"
        continue
    }

    $commands = @(
        "load $($test.Program)"
        "mreg v2.0 10"
        "mreg v2.1 20"
        "mreg v2.2 30"
        "mreg v2.3 40"
        "run"
        "regs"
        "pmem 100 4"
        "pmem 200 4"
        "info"
        "exit"
    )

    # Capture stdout and stderr together.
    # Native CLI diagnostic messages on stderr are not necessarily failures.
    $output = ($commands -join "`n") |
        & $exe --vm $test.Vm --repl 2>&1 |
        Out-String

    # Register checks
    $v1Ok = $output -match '(?m)^\s*v1\s*=\s*11,\s*22,\s*33,\s*44\s*$'
    $v2Ok = $output -match '(?m)^\s*v2\s*=\s*10,\s*20,\s*30,\s*40\s*$'
    $v3Ok = $output -match '(?m)^\s*v3\s*=\s*21,\s*42,\s*63,\s*84\s*$'
    $v4Ok = $output -match '(?m)^\s*v4\s*=\s*10,\s*20,\s*30,\s*40\s*$'

    # Memory at 0x100: 11, 22, 33, 44
    $mem100Checks = @(
        ($output -match '(?m)0x0000000000000100\s+\|.*0x000000000000000b'),
        ($output -match '(?m)0x0000000000000108\s+\|.*0x0000000000000016'),
        ($output -match '(?m)0x0000000000000110\s+\|.*0x0000000000000021'),
        ($output -match '(?m)0x0000000000000118\s+\|.*0x000000000000002c')
    )

    $mem100Ok = -not ($mem100Checks -contains $false)

    # Memory at 0x200: 10, 20, 30, 40
    $mem200Checks = @(
        ($output -match '(?m)0x0000000000000200\s+\|.*0x000000000000000a'),
        ($output -match '(?m)0x0000000000000208\s+\|.*0x0000000000000014'),
        ($output -match '(?m)0x0000000000000210\s+\|.*0x000000000000001e'),
        ($output -match '(?m)0x0000000000000218\s+\|.*0x0000000000000028')
    )

    $mem200Ok = -not ($mem200Checks -contains $false)

    $pass = (
        $v1Ok -and
        $v2Ok -and
        $v3Ok -and
        $v4Ok -and
        $mem100Ok -and
        $mem200Ok
    )

    # Performance statistics
    $cycles = "not found"
    $retired = "not found"
    $cpi = "not found"
    $stalls = "not found"

    if ($output -match '(?m)Cycles:\s*(\d+)') {
        $cycles = $Matches[1]
    }

    if ($output -match '(?m)Instructions retired:\s*(\d+)') {
        $retired = $Matches[1]
    }

    if ($output -match '(?m)CPI:\s*([0-9.]+)') {
        $cpi = $Matches[1]
    }

    if ($output -match '(?m)Stall cycles:\s*(\d+)') {
        $stalls = $Matches[1]
    }

    $result = if ($pass) { "PASS" } else { "FAIL" }

    Write-Host "$($test.Vm): $result" `
        -ForegroundColor $(if ($pass) { "Green" } else { "Red" })

    $summary.Add([PSCustomObject]@{
        VM = $test.Vm
        Result = $result
        Cycles = $cycles
        Retired = $retired
        CPI = $cpi
        Stalls = $stalls
    })

    $report.Add("=" * 78)
    $report.Add("$($test.Vm) - $($test.Label)")
    $report.Add("Program: $($test.Program)")
    $report.Add("Functional result: $result")
    $report.Add("v1=$v1Ok; v2=$v2Ok; v3=$v3Ok; v4=$v4Ok")
    $report.Add("Memory 0x100=$mem100Ok; Memory 0x200=$mem200Ok")
    $report.Add("Cycles=$cycles; Retired=$retired; CPI=$cpi; Stalls=$stalls")
    $report.Add("")
    $report.Add("----- Full CLI output -----")
    $report.Add($output)
    $report.Add("")
}

$report.Insert(0, "KITES VECTOR PIPELINE REGRESSION REPORT")
$report.Insert(1, "Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
$report.Insert(2, "Project: $projectRoot")
$report.Insert(3, "")

$report | Set-Content -Path $reportPath -Encoding UTF8

Write-Host ""
Write-Host "========== REGRESSION SUMMARY ==========" -ForegroundColor Cyan

$summary | Format-Table -AutoSize

Write-Host "Full report saved to:"
Write-Host $reportPath -ForegroundColor Green

if (($summary | Where-Object { $_.Result -ne "PASS" }).Count -gt 0) {
    exit 1
}

exit 0
