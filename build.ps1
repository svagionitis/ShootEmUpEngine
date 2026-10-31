# build.ps1 - Windows PowerShell build launcher for Shoot'Em-Up Engine (Thexder)
[CmdletBinding()]
param(
    [Parameter(Position=0)]
    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$Config = "Debug",

    [switch]$Clean,
    [switch]$Test,
    [switch]$Run,
    [string]$Target = $null
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

$PythonExe = Get-Command python3 -ErrorAction SilentlyContinue
if (-not $PythonExe) {
    $PythonExe = Get-Command python -ErrorAction SilentlyContinue
}

if ($PythonExe) {
    $PyArgs = @("scripts/build.py", "--config", $Config)
    if ($Clean)  { $PyArgs += "--clean" }
    if ($Test)   { $PyArgs += "--test" }
    if ($Run)    { $PyArgs += "--run" }
    if ($Target) { $PyArgs += @("--target", $Target) }

    & $PythonExe.Source @PyArgs
    exit $LASTEXITCODE
}

Write-Host "[INFO] Python not found; falling back to direct CMake invocation."
$BuildDir = "build"
if ($Clean -and (Test-Path $BuildDir)) {
    Remove-Item -Recurse -Force $BuildDir
}
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

$CmakeArgs = @("-S", ".", "-B", $BuildDir, "-DCMAKE_BUILD_TYPE=$Config", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
if ($env:VCPKG_ROOT -and (Test-Path "$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake")) {
    $CmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
} elseif (Test-Path "C:/vcpkg/scripts/buildsystems/vcpkg.cmake") {
    $CmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
}

cmake @CmakeArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$BuildArgs = @("--build", $BuildDir, "--config", $Config, "--parallel")
if ($Target) { $BuildArgs += @("--target", $Target) }
cmake @BuildArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($Test) {
    ctest --test-dir $BuildDir --output-on-failure
}
if ($Run) {
    & "$BuildDir/apps/ThexderGame/ThexderGame.exe"
}

