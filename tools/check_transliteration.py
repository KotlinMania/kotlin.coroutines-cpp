#!/usr/bin/env python
"""
Check transliteration quality between C++ and Kotlin source files.

This tool:
1. Scans for TODO comments in C++ source files
2. For each TODO, shows context around it
3. Finds the corresponding .kt file for comparison
4. Runs diff between the C++ and Kotlin sections

Usage:
    python tools/check_transliteration.py [--fail-on-todo] [path]

Exit codes:
    0 - No issues found
    1 - TODOs found (if --fail-on-todo is set)
    2 - Error during execution
"""

import os
import re
import sys
import argparse
import subprocess
from pathlib import Path
from typing import List, Tuple, Optional

# Project paths
PROJECT_ROOT = Path(__file__).parent.parent
SRC_DIR = PROJECT_ROOT / "src" / "kotlinx" / "coroutines"
KOTLIN_DIR = PROJECT_ROOT / "tmp" / "kotlinx.coroutines" / "kotlinx-coroutines-core"

# File extensions to scan
CPP_EXTENSIONS = {'.hpp', '.cpp', '.h'}
KOTLIN_EXTENSION = '.kt'

# TODO pattern - matches TODO with optional tag like TODO(port):
TODO_PATTERN = re.compile(r'//\s*TODO(\([^)]*\))?:\s*(.+)')

# Header pattern to find Kotlin source reference
KOTLIN_REF_PATTERN = re.compile(r'Transliterated from:\s*(.+\.kt)', re.IGNORECASE)
KOTLIN_LINE_PATTERN = re.compile(r'Line\s+(\d+)(?:-(\d+))?', re.IGNORECASE)


class TodoItem:
    """Represents a TODO comment found in the codebase."""
    def __init__(self, file_path: Path, line_num: int, tag: str, message: str, context: List[str]):
        self.file_path = file_path
        self.line_num = line_num
        self.tag = tag  # e.g., "port", "semantics", "suspend-plugin"
        self.message = message
        self.context = context  # Lines around the TODO

    def __str__(self):
        return f"{self.file_path}:{self.line_num}: TODO({self.tag}): {self.message}"


def find_kotlin_source(cpp_file: Path) -> Optional[Path]:
    """
    Find the corresponding Kotlin source file for a C++ file.
    Looks for 'Transliterated from:' comment in the file header.
    """
    try:
        with open(cpp_file, 'r', encoding='utf-8', errors='replace') as f:
            # Only read first 50 lines for header
            for i, line in enumerate(f):
                if i > 50:
                    break
                match = KOTLIN_REF_PATTERN.search(line)
                if match:
                    kt_path = match.group(1).strip()
                    # Try to find the file in KOTLIN_DIR
                    for root, dirs, files in os.walk(KOTLIN_DIR):
                        for fname in files:
                            if fname.endswith('.kt'):
                                full_path = Path(root) / fname
                                # Check if path matches
                                if kt_path in str(full_path):
                                    return full_path
                    # Try direct path construction
                    possible_paths = [
                        KOTLIN_DIR / "common" / "src" / kt_path,
                        KOTLIN_DIR / kt_path,
                    ]
                    for p in possible_paths:
                        if p.exists():
                            return p
    except Exception as e:
        print(f"Warning: Could not read {cpp_file}: {e}", file=sys.stderr)
    return None


def get_context_lines(file_path: Path, line_num: int, before: int = 5, after: int = 5) -> List[str]:
    """Get lines around a specific line number with line numbers prefixed."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            lines = f.readlines()

        start = max(0, line_num - before - 1)
        end = min(len(lines), line_num + after)

        context = []
        for i in range(start, end):
            prefix = ">>> " if i == line_num - 1 else "    "
            context.append(f"{prefix}{i+1:4d}: {lines[i].rstrip()}")
        return context
    except Exception:
        return []


def extract_line_reference(todo_message: str) -> Optional[Tuple[int, Optional[int]]]:
    """Extract Kotlin line reference from TODO message if present."""
    match = KOTLIN_LINE_PATTERN.search(todo_message)
    if match:
        start = int(match.group(1))
        end = int(match.group(2)) if match.group(2) else None
        return (start, end)
    return None


def get_kotlin_context(kt_file: Path, start_line: int, end_line: Optional[int] = None,
                       context_before: int = 3, context_after: int = 3) -> List[str]:
    """Get context from Kotlin file around specific lines."""
    try:
        with open(kt_file, 'r', encoding='utf-8', errors='replace') as f:
            lines = f.readlines()

        if end_line is None:
            end_line = start_line

        start = max(0, start_line - context_before - 1)
        end = min(len(lines), end_line + context_after)

        context = []
        for i in range(start, end):
            in_range = start_line - 1 <= i <= end_line - 1
            prefix = ">>> " if in_range else "    "
            context.append(f"{prefix}{i+1:4d}: {lines[i].rstrip()}")
        return context
    except Exception:
        return []


def scan_for_todos(directory: Path) -> List[TodoItem]:
    """Scan directory for TODO comments in C++ files."""
    todos = []

    for root, dirs, files in os.walk(directory):
        # Skip vendor, tmp, build directories
        dirs[:] = [d for d in dirs if d not in {'vendor', 'tmp', 'build', '.git', 'debugging'}]

        for fname in files:
            ext = Path(fname).suffix
            if ext not in CPP_EXTENSIONS:
                continue

            file_path = Path(root) / fname
            try:
                with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
                    lines = f.readlines()

                for i, line in enumerate(lines):
                    match = TODO_PATTERN.search(line)
                    if match:
                        tag = match.group(1) or ""
                        tag = tag.strip("()")
                        message = match.group(2).strip()
                        context = get_context_lines(file_path, i + 1)
                        todos.append(TodoItem(file_path, i + 1, tag, message, context))
            except Exception as e:
                print(f"Warning: Could not read {file_path}: {e}", file=sys.stderr)

    return todos


def print_todo_report(todos: List[TodoItem], verbose: bool = True):
    """Print a report of all TODOs found."""
    if not todos:
        print("No TODOs found.")
        return

    print(f"\n{'='*80}")
    print(f"TRANSLITERATION QUALITY REPORT")
    print(f"Found {len(todos)} TODO(s) in the codebase")
    print(f"{'='*80}\n")

    # Group by tag
    by_tag = {}
    for todo in todos:
        tag = todo.tag or "untagged"
        if tag not in by_tag:
            by_tag[tag] = []
        by_tag[tag].append(todo)

    # Print summary
    print("Summary by tag:")
    for tag, items in sorted(by_tag.items()):
        print(f"  {tag}: {len(items)}")
    print()

    if not verbose:
        # Just list locations
        for todo in todos:
            print(str(todo))
        return

    # Detailed report
    for todo in todos:
        print(f"\n{'-'*80}")
        print(f"FILE: {todo.file_path}")
        print(f"LINE: {todo.line_num}")
        print(f"TAG:  {todo.tag or 'none'}")
        print(f"MSG:  {todo.message}")
        print()

        # Show C++ context
        print("C++ Context:")
        for line in todo.context:
            print(f"  {line}")

        # Find and show Kotlin source
        kt_file = find_kotlin_source(todo.file_path)
        if kt_file:
            print(f"\nKotlin source: {kt_file}")

            # Try to find specific line reference
            line_ref = extract_line_reference(todo.message)
            if line_ref:
                start, end = line_ref
                kt_context = get_kotlin_context(kt_file, start, end)
                if kt_context:
                    print(f"Kotlin context (lines {start}-{end or start}):")
                    for line in kt_context:
                        print(f"  {line}")
        else:
            print("\nNo corresponding Kotlin source found.")

        print()


def main():
    parser = argparse.ArgumentParser(description="Check transliteration quality")
    parser.add_argument('path', nargs='?', default=str(SRC_DIR),
                        help='Path to scan (default: src/kotlinx/coroutines)')
    parser.add_argument('--fail-on-todo', action='store_true',
                        help='Exit with error if any TODOs are found')
    parser.add_argument('--summary', action='store_true',
                        help='Print only summary, not full details')
    parser.add_argument('--tag', type=str,
                        help='Filter by TODO tag (e.g., port, semantics)')

    args = parser.parse_args()

    scan_path = Path(args.path)
    if not scan_path.exists():
        print(f"Error: Path does not exist: {scan_path}", file=sys.stderr)
        return 2

    print(f"Scanning: {scan_path}")
    todos = scan_for_todos(scan_path)

    # Filter by tag if specified
    if args.tag:
        todos = [t for t in todos if t.tag == args.tag]

    print_todo_report(todos, verbose=not args.summary)

    if args.fail_on_todo and todos:
        print(f"\nBUILD HALTED: {len(todos)} TODO(s) found in codebase.")
        print("Fix all TODOs before proceeding with the build.")
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
