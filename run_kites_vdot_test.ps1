# KITES custom vdot.vv regression runner. Execute from project root:
#   .\run_kites_vdot_test.ps1

$ErrorActionPreference = "Continue"
if (Get-Variable PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$Root = $PSScriptRoot
$Exe = Join-Path $Root "build\debug\kites-cli.exe"
$Program = "examples\vdot_runtime_test.s"
$Report = Join-Path $Root "vdot_test_report.txt"
$Stderr = Join-Path $Root "vdot_test_stderr.txt"
if (-not (Test-Path $Exe)) {
    Write-Host "ERROR: KITES CLI not found: $Exe. Build the project first." -ForegroundColor Red
    exit 1
}

# Independently calculated encoding audit: funct6=010101, vs2=2, vs1=1,
# funct3=010, rd=5, opcode=0x57 => 0x5420a2d7.
$EncodingCommands = @("load examples\vdot_encoding_test.s", "disas", "exit")
$EncodingOutput = (($EncodingCommands -join "`n") + "`n") |
    & $Exe --vm rv5-h-f --repl 2> $Stderr | Out-String
if ($EncodingOutput -match '(?m)^\s*0\s*:\s*5420a2d7\s+vdot\.vv x5, v1, v2\s*$') {
    Write-Host "PASS: vdot.vv machine encoding = 0x5420a2d7" -ForegroundColor Green
} else {
    Write-Host "FAIL: encoding did not match expected 0x5420a2d7" -ForegroundColor Red
    Write-Host $EncodingOutput
    exit 1
}

$VMs = @("rv5-nh-nf", "rv5-nh-f", "rv5-h-nf", "rv5-h-f")
$AllPassed = $true
$ReportLines = [System.Collections.Generic.List[string]]::new()

foreach ($VM in $VMs) {
    Write-Host "`nTesting $VM ..." -ForegroundColor Cyan
    $Commands = @(
        "load $Program",
        "mreg v1.0 1", "mreg v1.1 2", "mreg v1.2 3", "mreg v1.3 4",
        "mreg v2.0 5", "mreg v2.1 6", "mreg v2.2 7", "mreg v2.3 8",
        "run", "regs", "exit"
    )
    $InputText = ($Commands -join "`n") + "`n"
    $Output = $InputText | & $Exe --vm $VM --repl 2> $Stderr | Out-String
    $ReportLines.Add("===== $VM =====")
    $ReportLines.Add($Output)

    $Checks = @(
        @{ Reg = "x6"; Value = 70; Label = "VL=4 dot result" },
        @{ Reg = "x7"; Value = 71; Label = "immediate scalar consumer" },
        @{ Reg = "x8"; Value = 17; Label = "VL=2 dot result" },
        @{ Reg = "x9"; Value = 0; Label = "VL=0 dot result" },
        @{ Reg = "x12"; Value = 70; Label = "first back-to-back dot" },
        @{ Reg = "x13"; Value = 70; Label = "second back-to-back dot" },
        @{ Reg = "x0"; Value = 0; Label = "x0 remains immutable" }
    )
    foreach ($Check in $Checks) {
        $Pattern = '(?m)^\s*' + [regex]::Escape($Check.Reg) + '\s*=\s*0x0*' + ([int]$Check.Value).ToString("x") + '\s*$'
        if ($Output -match $Pattern) {
            Write-Host "PASS [$VM]: $($Check.Label) = $($Check.Value)" -ForegroundColor Green
            $ReportLines.Add("PASS: $($Check.Label)")
        } else {
            Write-Host "FAIL [$VM]: expected $($Check.Reg)=$($Check.Value) ($($Check.Label))" -ForegroundColor Red
            $AllPassed = $false
            $ReportLines.Add("FAIL: $($Check.Label), expected $($Check.Reg)=$($Check.Value)")
        }
    }
}

# Immediate scalar dependency test: no NOPs between vdot and addi.
# Forwarding variants should bypass the dot result; H/NF should interlock.
$DependencyVMs = @("rv5-nh-f", "rv5-h-nf", "rv5-h-f")
foreach ($VM in $DependencyVMs) {
    Write-Host "`nTesting immediate dot-result dependency on $VM ..." -ForegroundColor Cyan
    $Commands = @(
        "load examples\vdot_immediate_dependency_test.s",
        "mreg v1.0 1", "mreg v1.1 2", "mreg v1.2 3", "mreg v1.3 4",
        "mreg v2.0 5", "mreg v2.1 6", "mreg v2.2 7", "mreg v2.3 8",
        "run", "regs", "exit"
    )
    $Output = (($Commands -join "`n") + "`n") | & $Exe --vm $VM --repl 2> $Stderr | Out-String
    $ReportLines.Add("===== immediate dependency $VM =====")
    $ReportLines.Add($Output)
    if ($Output -match '(?m)^\s*x7\s*=\s*0x0*47\s*$') {
        Write-Host "PASS [$VM]: immediate consumer x7=71" -ForegroundColor Green
        $ReportLines.Add("PASS: immediate dependency x7=71")
    } else {
        Write-Host "FAIL [$VM]: immediate consumer expected x7=71" -ForegroundColor Red
        $AllPassed = $false
        $ReportLines.Add("FAIL: immediate dependency x7 expected 71")
    }
}

$ReportLines | Set-Content -Path $Report
Write-Host "`nReport: $Report"
if ($AllPassed) {
    Write-Host "OVERALL RESULT: PASS" -ForegroundColor Green
    exit 0
} else {
    Write-Host "OVERALL RESULT: FAIL" -ForegroundColor Red
    exit 1
}