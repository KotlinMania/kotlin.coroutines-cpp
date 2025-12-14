#!/usr/bin/env python
"""
Build wrapper that provides porting context on compile errors.

When a compile error occurs, this script:
1. Parses the error to find the source file
2. Looks up the corresponding Kotlin source
3. Shows relevant context around the error
4. Lists any TODOs or stubs in the file

Usage:
    python tools/build_with_analysis.py [cmake-build-args...]

Example:
    python tools/build_with_analysis.py          # Build all
    python tools/build_with_analysis.py -j4      # Parallel build
"""

import os
import re
import sys
import subprocess
from pathlib import Path

ROOT = Path(__file__).parent.parent
KOTLIN_SRC = ROOT / "tmp" / "kotlinx.coroutines" / "kotlinx-coroutines-core"
CPP_SRC = ROOT / "src"
BUILD_DIR = ROOT / "build"

# Pattern to extract file:line from compiler errors
ERROR_PATTERN = re.compile(r'^(/[^:]+\.(cpp|hpp|h)):(\d+):\d+: (error|warning):', re.MULTILINE)

# Pattern for "Transliterated from:" header
KOTLIN_REF_PATTERN = re.compile(r'Transliterated from[:\s]+(.+\.kt)', re.IGNORECASE)

# TODO pattern
TODO_PATTERN = re.compile(r'//\s*TODO(\([^)]*\))?:\s*(.+)')


def find_kotlin_source(cpp_file: Path) -> Path | None:
    """Find corresponding Kotlin source from header comment."""
    try:
        with open(cpp_file, 'r', encoding='utf-8', errors='replace') as f:
            for i, line in enumerate(f):
                if i > 50:
                    break
                match = KOTLIN_REF_PATTERN.search(line)
                if match:
                    kt_path = match.group(1).strip()
                    # Search for the file
                    for root, dirs, files in os.walk(KOTLIN_SRC):
                        for fname in files:
                            if fname.endswith('.kt') and kt_path.endswith(fname):
                                return Path(root) / fname
                            if kt_path in str(Path(root) / fname):
                                return Path(root) / fname
    except Exception:
        pass

    # Try to guess from filename
    cpp_name = cpp_file.stem
    # Remove .common suffix if present
    if cpp_name.endswith('.common'):
        cpp_name = cpp_name[:-7]

    for root, dirs, files in os.walk(KOTLIN_SRC):
        for fname in files:
            if fname == f"{cpp_name}.kt":
                return Path(root) / fname

    return None


def get_context(file_path: Path, line_num: int, before: int = 5, after: int = 10) -> list[str]:
    """Get lines around the error with line numbers."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            lines = f.readlines()

        start = max(0, line_num - before - 1)
        end = min(len(lines), line_num + after)

        context = []
        for i in range(start, end):
            marker = ">>>" if i == line_num - 1 else "   "
            context.append(f"{marker} {i+1:4d}: {lines[i].rstrip()}")
        return context
    except Exception:
        return []


def find_todos_in_file(file_path: Path) -> list[tuple[int, str, str]]:
    """Find all TODOs in a file. Returns [(line, tag, message), ...]"""
    todos = []
    try:
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            for i, line in enumerate(f, 1):
                match = TODO_PATTERN.search(line)
                if match:
                    tag = match.group(1) or ""
                    tag = tag.strip("()")
                    message = match.group(2).strip()
                    todos.append((i, tag, message))
    except Exception:
        pass
    return todos


def count_lines(file_path: Path) -> int:
    """Count lines in a file."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            return sum(1 for _ in f)
    except Exception:
        return 0


def analyze_error(cpp_file: str, line_num: int, error_msg: str):
    """Provide context for a compile error."""
    cpp_path = Path(cpp_file)

    print("\n" + "=" * 80)
    print("PORTING ANALYSIS")
    print("=" * 80)

    print(f"\nC++ File: {cpp_path}")
    print(f"Error at line {line_num}")

    # Show C++ context
    cpp_context = get_context(cpp_path, line_num)
    if cpp_context:
        print(f"\nC++ Context:")
        for line in cpp_context:
            print(f"  {line}")

    # Find Kotlin source
    kt_path = find_kotlin_source(cpp_path)
    if kt_path:
        print(f"\nKotlin Source: {kt_path}")
        cpp_lines = count_lines(cpp_path)
        kt_lines = count_lines(kt_path)
        print(f"Line counts: C++ {cpp_lines} vs Kotlin {kt_lines}")
        if kt_lines > 0:
            ratio = cpp_lines / kt_lines
            if ratio < 0.8:
                print(f"  WARNING: C++ is only {ratio:.0%} of Kotlin - likely incomplete!")
    else:
        print("\nNo corresponding Kotlin source found.")

    # Find TODOs in the file
    todos = find_todos_in_file(cpp_path)
    if todos:
        print(f"\nTODOs in this file ({len(todos)}):")
        for line, tag, msg in todos[:10]:  # Show first 10
            tag_str = f"({tag})" if tag else ""
            print(f"  Line {line}: TODO{tag_str}: {msg}")
        if len(todos) > 10:
            print(f"  ... and {len(todos) - 10} more")

    # Check for stubs (functions that just throw or return default)
    stub_patterns = [
        r'throw\s+std::logic_error\s*\(\s*".*not.*implement',
        r'throw\s+std::runtime_error\s*\(\s*".*stub',
        r'return\s+nullptr;\s*//.*TODO',
        r'{\s*}\s*//.*TODO',
    ]

    try:
        with open(cpp_path, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()

        stubs = []
        for pattern in stub_patterns:
            for match in re.finditer(pattern, content, re.IGNORECASE):
                # Find line number
                line_num = content[:match.start()].count('\n') + 1
                stubs.append((line_num, match.group(0)[:60]))

        if stubs:
            print(f"\nPotential stubs found:")
            for line, snippet in stubs[:5]:
                print(f"  Line {line}: {snippet}...")
    except Exception:
        pass

    print("\n" + "=" * 80)


def main():
    # Change to build directory
    os.chdir(BUILD_DIR)

    # Build command
    cmd = ["cmake", "--build", "."] + sys.argv[1:]

    print(f"Running: {' '.join(cmd)}")
    print("-" * 80)

    # Run build and capture output
    result = subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
    )

    # Print build output
    print(result.stdout)

    # If failed, analyze errors
    if result.returncode != 0:
        errors = ERROR_PATTERN.findall(result.stdout)

        if errors:
            # Deduplicate by file
            seen_files = set()
            for file_path, ext, line_num, error_type in errors:
                if file_path not in seen_files and error_type == 'error':
                    seen_files.add(file_path)
                    analyze_error(file_path, int(line_num), "")
                    break  # Only analyze first error file

    return result.returncode


if __name__ == '__main__':
    sys.exit(main())
