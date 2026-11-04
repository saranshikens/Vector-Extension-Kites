# KITES vsetvli Undo/Redo regression test
# Run from project root:
# .\run_kites_vsetvli_undo_redo.ps1

$ErrorActionPreference = "Continue"

if (Get-Variable PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$Root = $PSScriptRoot
$Exe = Join-Path $Root "build\debug\kites-cli.exe"
$TestFile = Join-Path $Root "examples\vsetvli_undo_redo_test.s"
$ReportFile = Join-Path $Root "vsetvli_undo_redo_report.txt"
$StderrFile = Join-Path $Root "vsetvli_stderr.txt"

# Check executable
if (-not (Test-Path $Exe)) {
    Write-Host "ERROR: KITES CLI executable not found: $Exe" -ForegroundColor Red
    exit 1
}

# Create assembly test automatically
New-Item -ItemType Directory -Force -Path (Split-Path $TestFile) | Out-Null

@'
    vsetvli  x5, x10, 24
    vsetvli  x6, x11, 24
    vadd.vv  v3, v1, v2
'@ | Set-Content -Path $TestFile -Encoding ASCII

Write-Host "`nKITES vsetvli Undo/Redo Regression" -ForegroundColor Cyan
Write-Host "VM: rv5-h-f"
Write-Host "-----------------------------------"

# All operations are performed automatically in one REPL session.
$Commands = @(
    "load examples/vsetvli_undo_redo_test.s"

    "mreg x10 2"
    "mreg x11 4"

    "mreg v1.0 1"
    "mreg v1.1 2"
    "mreg v1.2 3"
    "mreg v1.3 4"

    "mreg v2.0 10"
    "mreg v2.1 20"
    "mreg v2.2 30"
    "mreg v2.3 40"

    # Both vsetvli instructions have executed.
    "step 4"
    "regs"

    # Undo the second vsetvli.
    "undo"
    "regs"

    # Redo the second vsetvli.
    "redo"
    "regs"

    # Complete the vector addition.
    "step 3"
    "regs"
    "info"

    "exit"
)

# Capture stdout and stderr separately.
$InputText = ($Commands -join "`n") + "`n"

$Output = $InputText |
    & $Exe --vm rv5-h-f --repl 2> $StderrFile |
    Out-String

$ExitCode = $LASTEXITCODE

$Output | Set-Content -Path $ReportFile

Write-Host "`nSIMULATOR OUTPUT" -ForegroundColor Cyan
Write-Host $Output

if ($ExitCode -ne 0) {
    Write-Host "FAIL: CLI exited with code $ExitCode" -ForegroundColor Red
    Write-Host "See $ReportFile"
    exit 1
}

$Passed = $true

function Check-Snapshot {
    param(
        [int]$Index,
        [string]$Expected,
        [string]$Description
    )

    if ($script:V1Lines.Count -lt $Index) {
        Write-Host "FAIL: Missing snapshot - $Description" -ForegroundColor Red
        $script:Passed = $false
        return
    }

    $Actual = $script:V1Lines[$Index - 1].Groups[1].Value
    $Actual = $Actual -replace '\s', ''

    if ($Actual -eq $Expected) {
        Write-Host "PASS: $Description ($Actual)" -ForegroundColor Green
    }
    else {
        Write-Host "FAIL: $Description - expected $Expected, got $Actual" -ForegroundColor Red
        $script:Passed = $false
    }
}

# Extract v1 from each register dump.
$V1Lines = [regex]::Matches(
    $Output,
    '(?m)^[ \t]*v1[ \t]*=[ \t]*(.+?)[ \t]*$'
)

Write-Host "`nUNDO / REDO CHECKS" -ForegroundColor Cyan

if ($V1Lines.Count -lt 4) {
    Write-Host "FAIL: Expected 4 register snapshots, found $($V1Lines.Count)" -ForegroundColor Red
    $Passed = $false
}
else {
    Check-Snapshot 1 "1,2,3,4" "Initial state after both vsetvli instructions: VL=4"
    Check-Snapshot 2 "1,2" "After Undo: VL restored to 2"
    Check-Snapshot 3 "1,2,3,4" "After Redo: VL restored to 4"
    Check-Snapshot 4 "1,2,3,4" "Final state: VL remains 4"
}

Write-Host "`nREGISTER CHECKS" -ForegroundColor Cyan

if ($Output -match '(?m)^\s*x5\s*=\s*0x0*2\s*$') {
    Write-Host "PASS: x5 = 2" -ForegroundColor Green
}
else {
    Write-Host "FAIL: x5 is not 2" -ForegroundColor Red
    $Passed = $false
}

if ($Output -match '(?m)^\s*x6\s*=\s*0x0*4\s*$') {
    Write-Host "PASS: x6 = 4" -ForegroundColor Green
}
else {
    Write-Host "FAIL: x6 is not 4" -ForegroundColor Red
    $Passed = $false
}

if ($Output -match '(?m)^\s*v3\s*=\s*11,\s*22,\s*33,\s*44\s*$') {
    Write-Host "PASS: v3 = 11, 22, 33, 44" -ForegroundColor Green
}
else {
    Write-Host "FAIL: Vector addition result is incorrect" -ForegroundColor Red
    $Passed = $false
}

Write-Host "`n-----------------------------------"

if ($Passed) {
    Write-Host "OVERALL RESULT: PASS" -ForegroundColor Green
}
else {
    Write-Host "OVERALL RESULT: FAIL" -ForegroundColor Red
}

Write-Host "Full report: $ReportFile"

if ((Test-Path $StderrFile) -and (Get-Item $StderrFile).Length -gt 0) {
    Write-Host "CLI diagnostics saved separately: $StderrFile"
}

if ($Passed) {
    exit 0
}
else {
    exit 1
}