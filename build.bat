@echo off
REM build.bat - Windows Command Prompt build launcher for Shoot'Em-Up Engine (Thexder)
setlocal EnableDelayedExpansion

cd /d "%~dp0"

where python >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    python scripts\build.py %*
    exit /b %ERRORLEVEL%
)

where python3 >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    python3 scripts\build.py %*
    exit /b %ERRORLEVEL%
)

echo [INFO] Python not found in PATH; falling back to native CMake execution.
if not exist "build" mkdir build

set CMAKE_ARGS=
if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
        set CMAKE_ARGS=-DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
    )
) else if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set CMAKE_ARGS=-DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
)

cmake -S . -B build %CMAKE_ARGS% -DCMAKE_BUILD_TYPE=Debug
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

cmake --build build --parallel
exit /b %ERRORLEVEL%

