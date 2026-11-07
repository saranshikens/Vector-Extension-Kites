# KITES vdot.vv GPR undo/redo test. Run from project root:
#   .\run_kites_vdot_undo_redo.ps1

$ErrorActionPreference = "Continue"
if (Get-Variable PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}
$Root = $PSScriptRoot
$Exe = Join-Path $Root "build\debug\kites-cli.exe"
$Program = "examples\vdot_undo_redo_test.s"
$Report = Join-Path $Root "vdot_undo_redo_report.txt"
if (-not (Test-Path $Exe)) {
    Write-Host "ERROR: KITES CLI not found: $Exe. Build first." -ForegroundColor Red
    exit 1
}
$Commands = @(
    "load $Program",
    "mreg v1.0 1", "mreg v1.1 2", "mreg v1.2 3", "mreg v1.3 4",
    "mreg v2.0 5", "mreg v2.1 6", "mreg v2.2 7", "mreg v2.3 8",
    "step 10", "regs", "undo", "regs", "redo", "regs", "exit"
)
$Output = (($Commands -join "`n") + "`n") | & $Exe --vm rv5-h-f 2> "$env:TEMP\kites_vdot_stderr.txt" | Out-String
$Output | Set-Content -Path $Report
Write-Host $Output

$Snapshots = [regex]::Matches($Output, '(?m)^\s*x6\s*=\s*(0x[0-9a-fA-F]+)\s*$')
$Passed = $Snapshots.Count -ge 3
if ($Passed) {
    $Expected = @("0x11", "0x0", "0x11")
    for ($i = 0; $i -lt 3; $i++) {
        if ($Snapshots[$i].Groups[1].Value -ne $Expected[$i]) { $Passed = $false }
    }
}
# VL is displayed as vector register lane count and x5 captures the chosen VL.
if ($Output -notmatch '(?m)^\s*v1\s*=\s*1,\s*2\s*$') { $Passed = $false }
if ($Output -notmatch '(?m)^\s*v2\s*=\s*5,\s*6\s*$') { $Passed = $false }
$VLRegisterSnapshots = [regex]::Matches($Output, '(?m)^\s*x5\s*=\s*0x0*2\s*$')
if ($VLRegisterSnapshots.Count -lt 3) { $Passed = $false }

if ($Passed) {
    Write-Host "OVERALL RESULT: PASS (x6=17 -> undo x6=0 -> redo x6=17; vector data/VL retained)" -ForegroundColor Green
    exit 0
} else {
    Write-Host "OVERALL RESULT: FAIL. See $Report" -ForegroundColor Red
    exit 1
}
