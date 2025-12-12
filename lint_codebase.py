#!/usr/bin/env python
"""
Lint the kotlinx.coroutines C++ port for structural issues.

Usage:
    python lint_codebase.py                    # Full report
    python lint_codebase.py --summary          # Summary only
    python lint_codebase.py --fix-preview      # Show what would be fixed
    python lint_codebase.py --json             # Output as JSON
"""

import os
import re
import sys
import json
import argparse
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).parent
KOTLIN_SRC = ROOT / "tmp" / "kotlinx.coroutines"
CPP_SRC = ROOT / "src"

class LintResult:
    def __init__(self):
        self.todos = []
        self.namespace_issues = []
        self.include_issues = []
        self.missing_files = []
        self.extra_files = []
        self.empty_stubs = []

    def to_dict(self):
        return {
            'todos': self.todos,
            'namespace_issues': self.namespace_issues,
            'include_issues': self.include_issues,
            'missing_files': self.missing_files,
            'extra_files': self.extra_files,
            'empty_stubs': self.empty_stubs,
        }

def count_todos(path):
    """Count TODO comments in a file."""
    todos = []
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            for i, line in enumerate(f, 1):
                if 'TODO' in line:
                    # Extract the TODO text
                    match = re.search(r'//\s*TODO[:\(]?\s*(.+)', line)
                    if match:
                        todos.append({
                            'file': str(path),
                            'line': i,
                            'text': match.group(1).strip()[:100]
                        })
    except:
        pass
    return todos

def check_namespace(path):
    """Check if file's namespace matches its folder location."""
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except:
        return None

    # Get expected namespace from path
    rel_path = path.relative_to(CPP_SRC)
    parts = list(rel_path.parent.parts)

    # Filter platform markers
    filtered = []
    for part in parts:
        if part in ('common', 'native', 'concurrent', 'src'):
            continue
        filtered.append(part)

    expected_ns = '.'.join(filtered) if filtered else None
    if not expected_ns or not expected_ns.startswith('kotlinx'):
        return None

    # Find declared namespace
    # Look for namespace kotlinx { namespace coroutines { ...
    declared = None
    ns_stack = []
    for line in content.split('\n'):
        # C++17 style
        m = re.match(r'\s*namespace\s+([\w:]+)\s*\{', line)
        if m and '::' in m.group(1):
            declared = m.group(1).replace('::', '.')
            break
        # Nested style
        m = re.match(r'\s*namespace\s+(\w+)\s*\{', line)
        if m and m.group(1) not in ('std', 'detail', 'anonymous', ''):
            ns_stack.append(m.group(1))
            if ns_stack and ns_stack[0] == 'kotlinx':
                declared = '.'.join(ns_stack)

    if declared and expected_ns and declared != expected_ns:
        return {
            'file': str(path),
            'expected': expected_ns,
            'declared': declared,
        }
    return None

def check_includes(cpp_path, kotlin_path):
    """Compare C++ includes vs Kotlin imports."""
    issues = []

    try:
        with open(cpp_path, 'r', encoding='utf-8', errors='ignore') as f:
            cpp_content = f.read()
        with open(kotlin_path, 'r', encoding='utf-8', errors='ignore') as f:
            kt_content = f.read()
    except:
        return issues

    # Extract Kotlin imports
    kt_imports = set()
    for m in re.finditer(r'^import\s+(kotlinx\.coroutines[\w.]*)', kt_content, re.MULTILINE):
        kt_imports.add(m.group(1))

    # Extract C++ includes
    cpp_includes = set()
    for m in re.finditer(r'#include\s*"(kotlinx/coroutines[^"]*)"', cpp_content):
        inc = m.group(1).replace('/', '.').replace('.hpp', '').replace('.h', '')
        cpp_includes.add(inc)

    # Find missing (in Kotlin but not C++)
    for kt_imp in kt_imports:
        # Normalize: kotlinx.coroutines.flow.* -> kotlinx.coroutines.flow
        base_pkg = re.sub(r'\.\*$', '', kt_imp)
        base_pkg = re.sub(r'\.[A-Z][a-zA-Z]*$', '', base_pkg)  # Remove class names

        if not any(cpp_inc.startswith(base_pkg) for cpp_inc in cpp_includes):
            issues.append({
                'cpp_file': str(cpp_path),
                'missing_import': kt_imp,
            })

    return issues

def find_empty_stubs(path):
    """Check if a .cpp file is essentially empty (just includes)."""
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except:
        return False

    # Remove comments
    content = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)

    # Remove includes
    content = re.sub(r'#include\s*[<"][^>"]*[>"]', '', content)

    # Remove namespace open/close
    content = re.sub(r'namespace\s+\w+\s*\{', '', content)
    content = re.sub(r'\}\s*//\s*namespace', '', content)

    # Remove pragmas
    content = re.sub(r'#pragma\s+\w+', '', content)

    # Check if anything substantial remains
    content = content.strip()
    # Allow for some template instantiations
    if len(content) < 50:
        return True
    return False

def map_kotlin_to_cpp():
    """Build a map of Kotlin files to expected C++ files."""
    mapping = {}

    for kt_file in KOTLIN_SRC.rglob("*.kt"):
        rel = kt_file.relative_to(KOTLIN_SRC)
        parts = list(rel.parts)

        # Skip test files for now
        if 'test' in str(rel).lower() and 'kotlinx-coroutines-test' not in str(rel):
            continue

        # Extract base name
        base = kt_file.stem
        base = base.replace('.common', '').replace('.native', '').replace('.concurrent', '')

        # Determine expected C++ location based on package
        try:
            with open(kt_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            match = re.search(r'^package\s+([\w.]+)', content, re.MULTILINE)
            if match:
                pkg = match.group(1)
                cpp_dir = pkg.replace('.', '/')
                cpp_path = CPP_SRC / cpp_dir / f"{base}.cpp"
                hpp_path = CPP_SRC / cpp_dir / f"{base}.hpp"

                mapping[str(kt_file)] = {
                    'kotlin': str(kt_file),
                    'package': pkg,
                    'expected_cpp': str(cpp_path),
                    'expected_hpp': str(hpp_path),
                    'cpp_exists': cpp_path.exists(),
                    'hpp_exists': hpp_path.exists(),
                }
        except:
            pass

    return mapping

def run_lint(args):
    result = LintResult()

    print("Scanning codebase...", file=sys.stderr)

    # Count TODOs
    todo_count = 0
    for ext in ['*.cpp', '*.hpp']:
        for path in CPP_SRC.rglob(ext):
            todos = count_todos(path)
            todo_count += len(todos)
            if not args.summary:
                result.todos.extend(todos)

    # Check namespaces
    ns_issues = 0
    for ext in ['*.cpp', '*.hpp']:
        for path in CPP_SRC.rglob(ext):
            if 'test' in str(path) or 'tools' in str(path):
                continue
            issue = check_namespace(path)
            if issue:
                ns_issues += 1
                if not args.summary:
                    result.namespace_issues.append(issue)

    # Check file mapping
    mapping = map_kotlin_to_cpp()
    missing = 0
    for kt_path, info in mapping.items():
        if not info['cpp_exists'] and not info['hpp_exists']:
            missing += 1
            if not args.summary:
                result.missing_files.append({
                    'kotlin': kt_path,
                    'package': info['package'],
                    'expected': info['expected_cpp'],
                })

    # Check empty stubs
    stubs = 0
    for path in CPP_SRC.rglob('*.cpp'):
        if find_empty_stubs(path):
            stubs += 1
            if not args.summary:
                result.empty_stubs.append(str(path))

    # Output
    if args.json:
        print(json.dumps(result.to_dict(), indent=2))
    else:
        print("=" * 70)
        print("KOTLINX.COROUTINES C++ PORT - LINT REPORT")
        print("=" * 70)
        print()
        print(f"  TODOs found:           {todo_count:>5}")
        print(f"  Namespace mismatches:  {ns_issues:>5}")
        print(f"  Missing C++ files:     {missing:>5}")
        print(f"  Empty stub files:      {stubs:>5}")
        print()

        if args.summary:
            print("Run without --summary for full details.")
        else:
            if result.namespace_issues:
                print("-" * 70)
                print("NAMESPACE ISSUES (first 20):")
                print("-" * 70)
                for issue in result.namespace_issues[:20]:
                    print(f"  {issue['file']}")
                    print(f"    expected: {issue['expected']}")
                    print(f"    declared: {issue['declared']}")
                if len(result.namespace_issues) > 20:
                    print(f"  ... and {len(result.namespace_issues) - 20} more")

            if result.missing_files:
                print()
                print("-" * 70)
                print("MISSING C++ FILES (first 20):")
                print("-" * 70)
                for item in result.missing_files[:20]:
                    print(f"  {item['package']}")
                    print(f"    kotlin: {item['kotlin']}")
                if len(result.missing_files) > 20:
                    print(f"  ... and {len(result.missing_files) - 20} more")

            if result.empty_stubs:
                print()
                print("-" * 70)
                print("EMPTY STUB FILES (first 20):")
                print("-" * 70)
                for path in result.empty_stubs[:20]:
                    print(f"  {path}")
                if len(result.empty_stubs) > 20:
                    print(f"  ... and {len(result.empty_stubs) - 20} more")

        print()
        print("=" * 70)

def main():
    parser = argparse.ArgumentParser(description='Lint kotlinx.coroutines C++ port')
    parser.add_argument('--summary', action='store_true', help='Show summary only')
    parser.add_argument('--json', action='store_true', help='Output as JSON')
    parser.add_argument('--fix-preview', action='store_true', help='Show what would be fixed')
    args = parser.parse_args()

    run_lint(args)

if __name__ == "__main__":
    main()
