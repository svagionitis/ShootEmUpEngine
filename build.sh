#!/usr/bin/env bash
# build.sh - Linux & POSIX build launcher for Shoot'Em-Up Engine (Thexder)
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if command -v python3 &>/dev/null; then
    exec python3 scripts/build.py "$@"
elif command -v python &>/dev/null; then
    exec python scripts/build.py "$@"
else
    echo "[INFO] Python 3 not detected; falling back to native CMake execution."
    BUILD_DIR="${BUILD_DIR:-build}"
    CONFIG="${1:-Debug}"
    
    mkdir -p "$BUILD_DIR"
    cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$CONFIG" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build "$BUILD_DIR" --parallel "$(nproc 2>/dev/null || echo 4)"
fi

