# KITES vsetvli regression test for all four five-stage pipeline modes.

$ErrorActionPreference = "Continue"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$exe = Join-Path $projectRoot "build\debug\kites-cli.exe"
$reportPath = Join-Path $projectRoot "vsetvli_regression_report.txt"

if (-not (Test-Path $exe)) {
    Write-Host "ERROR: CLI executable not found: $exe" -ForegroundColor Red
    Write-Host "Build the project first."
    exit 1
}

$vms = @("rv5-nh-nf", "rv5-nh-f", "rv5-h-nf", "rv5-h-f")
$summary = [System.Collections.Generic.List[object]]::new()
$report = [System.Collections.Generic.List[string]]::new()

foreach ($vm in $vms) {
    Write-Host "`nTesting $vm ..." -ForegroundColor Cyan

    # NH/NF has no forwarding or hazard detection, so use software scheduling.
    # The other three modes test the immediate dependency directly.
    $mainProgram = if ($vm -eq "rv5-nh-nf") {
        "examples\vsetvli_test_nh_nf.s"
    } else {
        "examples\vsetvli_test.s"
    }

    $mainCommands = @(
        "load $mainProgram",
        "mreg v1.0 1", "mreg v1.1 2", "mreg v1.2 3", "mreg v1.3 4",
        "mreg v2.0 10", "mreg v2.1 20", "mreg v2.2 30", "mreg v2.3 40",
        "run", "regs", "exit"
    )

    $mainOutput = ($mainCommands -join "`n") | & $exe --vm $vm --repl 2>&1 | Out-String

    $mainVlOk = $mainOutput -match '(?im)^\s*x5\s*=\s*0x0*2\s*$'
    $mainVectorOk = $mainOutput -match '(?im)^\s*v3\s*=\s*11,\s*22\s*$'

    $mainPass = $mainVlOk -and $mainVectorOk

    $summary.Add([PSCustomObject]@{
        VM = $vm
        Test = "AVL=2 + dependency + retain VL + vadd"
        Result = $(if ($mainPass) { "PASS" } else { "FAIL" })
    })

    $report.Add("=" * 72)
    $report.Add("$vm | Main test | x5=$mainVlOk; v3=$mainVectorOk")
    $report.Add($mainOutput)

    # AVL boundary tests: AVL=0 and AVL=100.
    foreach ($boundary in @(
        @{ Value = 0; Expected = "0" },
        @{ Value = 100; Expected = "40" }
    )) {
        $commands = @(
            "load examples\vsetvli_avl_test.s",
            "mreg x10 $($boundary.Value)",
            "run", "regs", "exit"
        )

        $output = ($commands -join "`n") | & $exe --vm $vm --repl 2>&1 | Out-String

        $vlOk = $output -match (
            '(?im)^\s*x5\s*=\s*0x0*' + $boundary.Expected + '\s*$'
        )

        $summary.Add([PSCustomObject]@{
            VM = $vm
            Test = "AVL=$($boundary.Value)"
            Result = $(if ($vlOk) { "PASS" } else { "FAIL" })
        })

        $report.Add("=" * 72)
        $report.Add("$vm | AVL=$($boundary.Value) | VL check=$vlOk")
        $report.Add($output)
    }

    # rs1=x0 and rd!=x0 must select VLMAX=64.
    $maxCommands = @(
        "load examples\vsetvli_vlmax_test.s",
        "run", "regs", "exit"
    )

    $maxOutput = ($maxCommands -join "`n") | & $exe --vm $vm --repl 2>&1 | Out-String
    $maxOk = $maxOutput -match '(?im)^\s*x5\s*=\s*0x0*40\s*$'

    $summary.Add([PSCustomObject]@{
        VM = $vm
        Test = "rs1=x0 selects VLMAX"
        Result = $(if ($maxOk) { "PASS" } else { "FAIL" })
    })

    $report.Add("=" * 72)
    $report.Add("$vm | rs1=x0 selects VLMAX | VL check=$maxOk")
    $report.Add($maxOutput)
}

$report.Insert(0, "KITES VSETVLI REGRESSION REPORT")
$report.Insert(1, "Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
$report | Set-Content -Path $reportPath -Encoding UTF8

Write-Host "`n========== VSETVLI TEST SUMMARY ==========" -ForegroundColor Cyan
$summary | Format-Table -AutoSize

Write-Host "Full report saved to: $reportPath"

$failed = ($summary | Where-Object { $_.Result -ne "PASS" }).Count

if ($failed -gt 0) {
    Write-Host "`nRESULT: $failed test(s) failed." -ForegroundColor Red
    exit 1
}

Write-Host "`nRESULT: ALL TESTS PASSED." -ForegroundColor Green
exit 0