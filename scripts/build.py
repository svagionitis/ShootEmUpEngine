#!/usr/bin/env python3
"""
build.py - Unified Cross-Platform Build Script for Windows and Linux.

Configures, builds, tests, and optionally runs the Shoot'Em-Up Engine (Thexder)
with automated platform detection, vcpkg/toolchain resolution, and parallel compilation.

Usage:
    python3 scripts/build.py [options]

Examples:
    python3 scripts/build.py
    python3 scripts/build.py --config Release
    python3 scripts/build.py --clean --test
    python3 scripts/build.py --run
    python3 scripts/build.py --target ThexderGame
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
from typing import List, Optional


def get_project_root() -> str:
    """Return the absolute path to the repository root directory."""
    return os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


def find_vcpkg_toolchain() -> Optional[str]:
    """Locate vcpkg.cmake on Windows or Linux if available."""
    # 1. Environment variable VCPKG_ROOT
    vcpkg_root = os.environ.get("VCPKG_ROOT")
    if vcpkg_root:
        candidate = os.path.join(vcpkg_root, "scripts", "buildsystems", "vcpkg.cmake")
        if os.path.isfile(candidate):
            return candidate

    # 2. Common Windows paths
    if platform.system() == "Windows":
        win_candidates = [
            r"C:\vcpkg\scripts\buildsystems\vcpkg.cmake",
            r"D:\vcpkg\scripts\buildsystems\vcpkg.cmake",
        ]
        for path in win_candidates:
            if os.path.isfile(path):
                return path

    # 3. Common Linux / POSIX paths
    home = os.path.expanduser("~")
    posix_candidates = [
        os.path.join(home, "vcpkg", "scripts", "buildsystems", "vcpkg.cmake"),
        "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake",
        "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake",
    ]
    for path in posix_candidates:
        if os.path.isfile(path):
            return path

    return None


def select_cmake_generator() -> Optional[str]:
    """Determine the optimal CMake generator for the active environment."""
    if shutil.which("ninja"):
        return "Ninja"
    if platform.system() != "Windows" and shutil.which("make"):
        return "Unix Makefiles"
    return None


def run_command(cmd: List[str], cwd: str, description: str) -> bool:
    """Run a subprocess command with clear status output."""
    print(f"\n[BUILD] >>> {description}")
    print(f"[CMD]   {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        print(f"\n[BUILD] FAILED: {description} (exit code: {result.returncode})", file=sys.stderr)
        return False
    return True


def configure_cmake(
    root_dir: str,
    build_dir: str,
    build_type: str,
    generator: Optional[str],
    toolchain_file: Optional[str],
    extra_cmake_args: List[str],
) -> bool:
    """Execute CMake configuration step."""
    os.makedirs(build_dir, exist_ok=True)

    cmake_bin = shutil.which("cmake")
    if not cmake_bin:
        print("[ERROR] CMake executable not found in PATH.", file=sys.stderr)
        return False

    cmd = [
        cmake_bin,
        "-S", root_dir,
        "-B", build_dir,
        f"-DCMAKE_BUILD_TYPE={build_type}",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    ]

    if generator:
        cmd.extend(["-G", generator])

    if toolchain_file and os.path.isfile(toolchain_file):
        cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}")

    cmd.extend(extra_cmake_args)

    return run_command(cmd, cwd=root_dir, description=f"Configuring CMake ({build_type})")


def build_cmake(
    root_dir: str,
    build_dir: str,
    build_type: str,
    target: Optional[str],
    jobs: int,
) -> bool:
    """Execute CMake build step."""
    cmake_bin = shutil.which("cmake")
    if not cmake_bin:
        return False

    cmd = [
        cmake_bin,
        "--build", build_dir,
        "--config", build_type,
        "--parallel", str(jobs),
    ]

    if target:
        cmd.extend(["--target", target])

    desc = f"Building target '{target}'" if target else "Building project"
    return run_command(cmd, cwd=root_dir, description=desc)


def run_tests(root_dir: str, build_dir: str, build_type: str) -> bool:
    """Run CTest automated test suite."""
    ctest_bin = shutil.which("ctest")
    if not ctest_bin:
        print("[WARNING] CTest executable not found; skipping tests.", file=sys.stderr)
        return True

    cmd = [
        ctest_bin,
        "--test-dir", build_dir,
        "--build-config", build_type,
        "--output-on-failure",
    ]

    return run_command(cmd, cwd=root_dir, description="Executing CTest Suite")


def locate_executable(build_dir: str, target_name: str) -> Optional[str]:
    """Locate the built game executable across build layouts."""
    exe_name = f"{target_name}.exe" if platform.system() == "Windows" else target_name

    candidates = [
        os.path.join(build_dir, "apps", target_name, exe_name),
        os.path.join(build_dir, "apps", target_name, "Debug", exe_name),
        os.path.join(build_dir, "apps", target_name, "Release", exe_name),
        os.path.join(build_dir, "bin", exe_name),
        os.path.join(build_dir, exe_name),
    ]

    for path in candidates:
        if os.path.isfile(path):
            return path
    return None


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Cross-platform build script for Shoot'Em-Up Engine (Windows & Linux)."
    )
    parser.add_argument(
        "-c", "--config",
        choices=["Debug", "Release", "RelWithDebInfo"],
        default="Debug",
        help="CMake build type (default: Debug).",
    )
    parser.add_argument(
        "-b", "--build-dir",
        default="build",
        help="Relative or absolute build directory path (default: 'build').",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Wipe the build directory before building.",
    )
    parser.add_argument(
        "-t", "--target",
        default=None,
        help="Specific build target (e.g. ThexderGame, format, check-format).",
    )
    parser.add_argument(
        "-j", "--jobs",
        type=int,
        default=os.cpu_count() or 4,
        help="Number of concurrent compilation jobs (default: CPU core count).",
    )
    parser.add_argument(
        "--test",
        action="store_true",
        help="Execute CTest suite upon successful compilation.",
    )
    parser.add_argument(
        "--run",
        action="store_true",
        help="Launch ThexderGame upon successful compilation.",
    )
    parser.add_argument(
        "--toolchain",
        default=None,
        help="Custom CMAKE_TOOLCHAIN_FILE path.",
    )
    parser.add_argument(
        "cmake_args",
        nargs=argparse.REMAINDER,
        help="Additional arguments passed directly to CMake configure.",
    )

    args = parser.parse_args()
    root_dir = get_project_root()
    build_dir = os.path.join(root_dir, args.build_dir) if not os.path.isabs(args.build_dir) else args.build_dir

    print("============================================================")
    print(" Shoot'Em-Up Engine (Thexder) - Cross-Platform Build Tool")
    print(f" Platform: {platform.system()} ({platform.machine()})")
    print(f" Root Directory:  {root_dir}")
    print(f" Build Directory: {build_dir}")
    print(f" Configuration:   {args.config}")
    print(f" Parallel Jobs:   {args.jobs}")
    print("============================================================")

    # 1. Clean build directory if requested
    if args.clean and os.path.isdir(build_dir):
        print(f"\n[BUILD] Cleaning build directory: {build_dir}")
        shutil.rmtree(build_dir)

    # 2. Toolchain and generator resolution
    toolchain = args.toolchain or find_vcpkg_toolchain()
    if toolchain:
        print(f"[INFO] Using toolchain file: {toolchain}")
    else:
        print("[INFO] No external toolchain specified; using native environment packages.")

    generator = select_cmake_generator()
    if generator:
        print(f"[INFO] Using CMake generator: {generator}")

    # 3. Configure
    if not configure_cmake(
        root_dir=root_dir,
        build_dir=build_dir,
        build_type=args.config,
        generator=generator,
        toolchain_file=toolchain,
        extra_cmake_args=args.cmake_args,
    ):
        return 1

    # 4. Build
    if not build_cmake(
        root_dir=root_dir,
        build_dir=build_dir,
        build_type=args.config,
        target=args.target,
        jobs=args.jobs,
    ):
        return 1

    # 5. Optional Tests
    if args.test:
        if not run_tests(root_dir=root_dir, build_dir=build_dir, build_type=args.config):
            return 1

    print("\n============================================================")
    print(" BUILD SUCCESSFUL")
    print("============================================================")

    # 6. Optional Run
    if args.run:
        exe_path = locate_executable(build_dir, "ThexderGame")
        if exe_path:
            print(f"\n[RUN] Launching: {exe_path}")
            return subprocess.run([exe_path], cwd=root_dir).returncode
        else:
            print("[ERROR] ThexderGame executable not found in build directory.", file=sys.stderr)
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())

