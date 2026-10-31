#!/usr/bin/env python3
"""
verify_clang_format.py - Automated verification and formatting using clang-format.

Performs code style verification and in-place formatting on C++ source and header files
to ensure repository-wide compliance with .clang-format.

Usage:
    python3 scripts/verify_clang_format.py [options] [paths...]

Options:
    --files FILE...     Verify or format specific files only.
    --fix               Format files in-place using clang-format -i.
    --dry-run           Verify formatting without modifying files (default).
    -v, --verbose       Show verbose diagnostic output.
"""

import argparse
import os
import shutil
import subprocess
import sys
from typing import List, Optional

CPP_EXTENSIONS = {".h", ".hpp", ".c", ".cc", ".cpp", ".cxx"}


def find_clang_format() -> Optional[str]:
    """Locate the clang-format executable on the current system."""
    candidate = shutil.which("clang-format")
    if candidate:
        return candidate

    # Common Windows installation directories
    windows_candidates = [
        r"C:\Program Files\LLVM\bin\clang-format.exe",
        r"C:\Program Files (x86)\LLVM\bin\clang-format.exe",
    ]
    for win_path in windows_candidates:
        if os.path.isfile(win_path):
            return win_path

    # Linux versioned binary names
    for ver in range(25, 14, -1):
        candidate = shutil.which(f"clang-format-{ver}")
        if candidate:
            return candidate

    return None


def collect_cpp_files(paths: List[str]) -> List[str]:
    """Collect C++ source and header files from specified paths."""
    collected = []
    for path in paths:
        if os.path.isfile(path):
            _, ext = os.path.splitext(path)
            if ext.lower() in CPP_EXTENSIONS:
                collected.append(os.path.normpath(path))
        elif os.path.isdir(path):
            for root, dirs, files in os.walk(path):
                # Skip build, .git, and vcpkg directories
                dirs[:] = [d for d in dirs if d not in {".git", "build", "vcpkg", "vcpkg_installed", "out"}]
                for file in files:
                    _, ext = os.path.splitext(file)
                    if ext.lower() in CPP_EXTENSIONS:
                        collected.append(os.path.normpath(os.path.join(root, file)))
    return sorted(list(set(collected)))


def run_clang_format(clang_format_bin: str, files: List[str], fix: bool, verbose: bool) -> int:
    """Run clang-format to either format in-place or check formatting."""
    violations: List[str] = []
    mode_str = "Formatting in-place" if fix else "Verifying formatting"

    if verbose:
        print(f"[clang-format] {mode_str} across {len(files)} file(s)...")

    for file_path in files:
        if fix:
            cmd = [clang_format_bin, "-i", file_path]
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            if result.returncode != 0:
                print(f"ERROR: Failed to format {file_path}:\n{result.stderr}", file=sys.stderr)
                violations.append(file_path)
            elif verbose:
                print(f"Formatted: {file_path}")
        else:
            cmd = [clang_format_bin, "--dry-run", "--Werror", file_path]
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            if result.returncode != 0:
                violations.append(file_path)
                if verbose and result.stderr:
                    print(f"Violation in {file_path}:\n{result.stderr.strip()}", file=sys.stderr)

    if violations:
        if fix:
            print(f"\n[clang-format] Encountered errors formatting {len(violations)} file(s).", file=sys.stderr)
        else:
            print(f"\n[clang-format] Style violations detected in {len(violations)} file(s):", file=sys.stderr)
            for v in violations:
                print(f"  - {v}", file=sys.stderr)
            print("\nRemediation: Run `python3 scripts/verify_clang_format.py --fix --files <file>` or `clang-format -i <file>`", file=sys.stderr)
        return 1

    print(f"[clang-format] SUCCESS: All {len(files)} file(s) adhere to repository style.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Verify or apply clang-format formatting for C++ files.")
    parser.add_argument("paths", nargs="*", default=["libs", "apps", "tests"], help="Directories or files to check (default: libs, apps, tests).")
    parser.add_argument("--files", nargs="+", help="Specific files to verify or format.")
    parser.add_argument("--fix", action="store_true", help="Format files in-place using clang-format -i.")
    parser.add_argument("--dry-run", action="store_true", help="Verify formatting without modifying files (default).")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose diagnostic output.")

    args = parser.parse_args()

    clang_format_bin = find_clang_format()
    if not clang_format_bin:
        print("ERROR: clang-format executable not found on PATH or standard install paths.", file=sys.stderr)
        return 1

    target_files = []
    if args.files:
        target_files = collect_cpp_files(args.files)
    else:
        target_files = collect_cpp_files(args.paths)

    if not target_files:
        if args.verbose:
            print("[clang-format] No C++ files detected.")
        return 0

    return run_clang_format(clang_format_bin, target_files, args.fix, args.verbose)


if __name__ == "__main__":
    sys.exit(main())

