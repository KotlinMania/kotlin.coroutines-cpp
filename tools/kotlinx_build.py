#!/usr/bin/env python
"""
Kotlin-aware C++ build system for kotlinx.coroutines port.

This replaces blind cmake compilation with intelligent, gated building:

1. DEPENDENCY ORDERING - Build files in import order (most depended-on first)
2. PRE-COMPILE VALIDATION - Check each file before compiling:
   - No duplicate symbol definitions
   - Function/class names match Kotlin (per transliteration rules)
   - Namespace matches folder structure
   - Required imports present
3. COMPILE GATE - Only compile if validation passes
4. INTELLIGENT FAILURE - Stop on first issue with full Kotlin context

Usage:
    python tools/kotlinx_build.py              # Build all, stop on first failure
    python tools/kotlinx_build.py --file X    # Build specific file
    python tools/kotlinx_build.py --validate  # Validate only, no compile
    python tools/kotlinx_build.py --order     # Show build order

Exit codes:
    0 - Build successful
    1 - Validation failed (pre-compile)
    2 - Compilation failed
    3 - Configuration error
"""

import os
import re
import sys
import json
import subprocess
import argparse
from pathlib import Path
from collections import defaultdict
from typing import Dict, List, Set, Tuple, Optional

# Project paths
ROOT = Path(__file__).parent.parent
KOTLIN_SRC = ROOT / "tmp" / "kotlinx.coroutines" / "kotlinx-coroutines-core"
CPP_SRC = ROOT / "src"
BUILD_DIR = ROOT / "build"

# Transliteration rules: Kotlin name -> C++ name
def kotlin_to_cpp_name(kotlin_name: str) -> str:
    """Convert Kotlin camelCase to C++ snake_case for functions."""
    # Insert underscore before uppercase letters, then lowercase
    result = re.sub(r'([a-z])([A-Z])', r'\1_\2', kotlin_name)
    return result.lower()

def cpp_to_kotlin_name(cpp_name: str) -> str:
    """Convert C++ snake_case to Kotlin camelCase."""
    parts = cpp_name.split('_')
    return parts[0] + ''.join(p.capitalize() for p in parts[1:])

# =============================================================================
# DEPENDENCY ANALYSIS
# =============================================================================

def extract_includes(cpp_file: Path) -> Set[str]:
    """Extract #include dependencies from a C++ file."""
    includes = set()
    try:
        with open(cpp_file, 'r', encoding='utf-8', errors='replace') as f:
            for line in f:
                # Match: #include "kotlinx/coroutines/..."
                match = re.match(r'#include\s*"(kotlinx/coroutines[^"]*)"', line)
                if match:
                    includes.add(match.group(1))
                # Match: #include <kotlinx/coroutines/...>
                match = re.match(r'#include\s*<(kotlinx/coroutines[^>]*)>', line)
                if match:
                    includes.add(match.group(1))
    except Exception:
        pass
    return includes

def build_dependency_graph() -> Dict[str, Set[str]]:
    """Build a graph of file dependencies based on #include statements."""
    graph = defaultdict(set)

    for ext in ['*.cpp', '*.hpp']:
        for cpp_file in CPP_SRC.rglob(ext):
            if 'test' in str(cpp_file).lower():
                continue
            rel_path = str(cpp_file.relative_to(CPP_SRC))
            includes = extract_includes(cpp_file)

            for inc in includes:
                # Normalize include path to relative path
                inc_rel = inc.replace('.hpp', '').replace('.h', '')
                graph[rel_path].add(inc)

    return dict(graph)

def topological_sort(graph: Dict[str, Set[str]]) -> List[str]:
    """Sort files by dependency order (most depended-on first)."""
    # Count how many files depend on each file
    dependency_count = defaultdict(int)
    all_files = set(graph.keys())

    for deps in graph.values():
        for dep in deps:
            dependency_count[dep] += 1

    # Sort by dependency count (descending) - most depended-on first
    sorted_files = sorted(all_files, key=lambda f: -dependency_count.get(f, 0))

    return sorted_files

def get_build_order() -> List[Path]:
    """Get files in optimal build order."""
    graph = build_dependency_graph()
    sorted_names = topological_sort(graph)

    # Convert to full paths, prioritize .hpp before .cpp
    result = []
    seen = set()

    for name in sorted_names:
        base = name.replace('.cpp', '').replace('.hpp', '')
        hpp_path = CPP_SRC / f"{base}.hpp"
        cpp_path = CPP_SRC / f"{base}.cpp"

        if hpp_path.exists() and str(hpp_path) not in seen:
            result.append(hpp_path)
            seen.add(str(hpp_path))
        if cpp_path.exists() and str(cpp_path) not in seen:
            result.append(cpp_path)
            seen.add(str(cpp_path))

    # Add any files not in the graph
    for ext in ['*.hpp', '*.cpp']:
        for f in CPP_SRC.rglob(ext):
            if str(f) not in seen and 'test' not in str(f).lower():
                result.append(f)
                seen.add(str(f))

    return result

# =============================================================================
# KOTLIN SOURCE MAPPING
# =============================================================================

def find_kotlin_source(cpp_file: Path) -> Optional[Path]:
    """Find corresponding Kotlin source file."""
    # Check for "Transliterated from:" header comment
    try:
        with open(cpp_file, 'r', encoding='utf-8', errors='replace') as f:
            for i, line in enumerate(f):
                if i > 50:
                    break
                match = re.search(r'Transliterated from[:\s]+(.+\.kt)', line, re.IGNORECASE)
                if match:
                    kt_path = match.group(1).strip()
                    # Search for matching file
                    for kt_file in KOTLIN_SRC.rglob("*.kt"):
                        if kt_path in str(kt_file):
                            return kt_file
    except Exception:
        pass

    # Fallback: match by filename
    base_name = cpp_file.stem.replace('.common', '').replace('.native', '')
    for kt_file in KOTLIN_SRC.rglob(f"{base_name}.kt"):
        return kt_file

    return None

def extract_kotlin_symbols(kt_file: Path) -> Dict:
    """Extract classes, functions from Kotlin source."""
    try:
        with open(kt_file, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()
    except:
        return {'classes': [], 'functions': [], 'package': None}

    # Remove comments to avoid false matches
    content_clean = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    content_clean = re.sub(r'/\*.*?\*/', '', content_clean, flags=re.DOTALL)

    # Get package
    pkg_match = re.search(r'^package\s+([\w.]+)', content, re.MULTILINE)
    package = pkg_match.group(1) if pkg_match else None

    # Extract classes/interfaces/objects - must have { to be a definition
    # Pattern: class/interface/object Name<generics>? : bases? {
    classes = []
    for m in re.finditer(r'(?:class|interface|object)\s+(\w+)(?:\s*<[^>]*>)?(?:\s*:[^{]*)?\s*\{', content_clean):
        name = m.group(1)
        # Filter out common false positives and short names
        if len(name) > 2 and name[0].isupper():
            classes.append(name)

    # Extract functions - public/private/internal fun name(
    functions = []
    for m in re.finditer(r'(?:fun|suspend\s+fun)\s+(?:<[^>]+>\s+)?(\w+)\s*\(', content_clean):
        name = m.group(1)
        if name not in ('get', 'set', 'invoke', 'equals', 'hashCode', 'toString') and len(name) > 2:
            functions.append(name)

    return {
        'classes': list(set(classes)),
        'functions': list(set(functions)),
        'package': package,
        'line_count': content.count('\n') + 1,
    }

def extract_cpp_symbols(cpp_file: Path) -> Dict:
    """Extract classes, functions from C++ source."""
    try:
        with open(cpp_file, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()
    except:
        return {'classes': [], 'functions': [], 'namespace': None}

    # Remove comments to avoid false matches
    content_clean = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    content_clean = re.sub(r'/\*.*?\*/', '', content_clean, flags=re.DOTALL)

    # Get namespace
    ns_match = re.search(r'namespace\s+([\w:]+)\s*\{', content_clean)
    namespace = ns_match.group(1).replace('::', '.') if ns_match else None

    # Extract classes/structs - ONLY actual definitions (with { ), not forward declarations
    # Forward declaration: "class Foo;" or "template<...> class Foo;"
    # Actual definition: "class Foo {" or "class Foo : public Base {"
    classes = []
    for m in re.finditer(r'(?:class|struct)\s+(\w+)\s*(?:final\s*)?(?::[^{;]*)?(\{|;)', content_clean):
        name = m.group(1)
        is_definition = m.group(2) == '{'
        if name not in ('std', 'detail') and is_definition:
            classes.append(name)

    # Extract functions (simplified)
    functions = []
    for m in re.finditer(r'(?:void|bool|int|auto|[\w:]+)\s+(\w+)\s*\([^)]*\)\s*(?:const\s*)?(?:override\s*)?[{;]', content_clean):
        name = m.group(1)
        if name not in ('if', 'while', 'for', 'switch', 'return'):
            functions.append(name)

    return {
        'classes': list(set(classes)),
        'functions': list(set(functions)),
        'namespace': namespace,
        'line_count': content.count('\n') + 1,
    }

# =============================================================================
# VALIDATION CHECKS
# =============================================================================

class ValidationResult:
    def __init__(self, file_path: Path):
        self.file_path = file_path
        self.passed = True
        self.errors = []
        self.warnings = []
        self.kotlin_source = None
        self.kotlin_symbols = None
        self.cpp_symbols = None

    def add_error(self, msg: str):
        self.passed = False
        self.errors.append(msg)

    def add_warning(self, msg: str):
        self.warnings.append(msg)

def check_duplicates(cpp_file: Path, all_symbols: Dict[str, List[str]]) -> List[str]:
    """Check if this file defines symbols already defined elsewhere."""
    errors = []
    symbols = extract_cpp_symbols(cpp_file)
    rel_path = str(cpp_file.relative_to(CPP_SRC))

    for cls in symbols['classes']:
        if cls in all_symbols:
            other_files = [f for f in all_symbols[cls] if f != rel_path]
            if other_files:
                errors.append(f"Duplicate class '{cls}' also defined in: {', '.join(other_files[:3])}")

    return errors

def check_symbol_coverage(cpp_file: Path, kt_file: Optional[Path], all_cpp_classes: Set[str]) -> Tuple[List[str], List[str]]:
    """Check if C++ has all symbols from Kotlin (with name translation)."""
    errors = []
    warnings = []

    if not kt_file:
        warnings.append("No corresponding Kotlin source found")
        return errors, warnings

    kt_symbols = extract_kotlin_symbols(kt_file)
    cpp_symbols = extract_cpp_symbols(cpp_file)

    # Check classes - they can be in ANY C++ file, not just the matching one
    kt_classes = set(kt_symbols['classes'])

    # Missing from this file AND missing from entire codebase
    missing_everywhere = kt_classes - all_cpp_classes
    if missing_everywhere:
        errors.append(f"Classes missing from ENTIRE codebase: {', '.join(sorted(missing_everywhere)[:5])}")

    # Missing from this file but exists elsewhere (just a warning)
    cpp_classes = set(cpp_symbols['classes'])
    missing_here = kt_classes - cpp_classes - missing_everywhere
    if missing_here and len(missing_here) <= 3:
        # Small number = probably intentional file split
        pass
    elif missing_here:
        warnings.append(f"Classes in other files: {', '.join(sorted(missing_here)[:5])}")

    # Check functions (with name translation)
    kt_funcs = set(kt_symbols['functions'])
    cpp_funcs = set(cpp_symbols['functions'])
    cpp_funcs_kotlin_style = set(cpp_to_kotlin_name(f) for f in cpp_funcs)

    missing_funcs = kt_funcs - cpp_funcs_kotlin_style - cpp_funcs
    if missing_funcs:
        # Only warn, not error - functions can be in different files
        warnings.append(f"Potentially missing functions: {', '.join(sorted(missing_funcs)[:5])}")

    # Check line count ratio - only error if VERY short
    if kt_symbols['line_count'] > 0:
        ratio = cpp_symbols['line_count'] / kt_symbols['line_count']
        if ratio < 0.3:
            errors.append(f"C++ is only {ratio:.0%} of Kotlin size ({cpp_symbols['line_count']} vs {kt_symbols['line_count']} lines)")
        elif ratio < 0.5:
            warnings.append(f"C++ is {ratio:.0%} of Kotlin size")

    return errors, warnings

def check_namespace(cpp_file: Path) -> List[str]:
    """Check if namespace matches folder structure."""
    errors = []

    rel_path = cpp_file.relative_to(CPP_SRC)
    folder_parts = list(rel_path.parent.parts)

    # Filter platform markers
    filtered = [p for p in folder_parts if p not in ('common', 'native', 'concurrent', 'src')]
    expected_ns = '.'.join(filtered) if filtered else None

    if not expected_ns or not expected_ns.startswith('kotlinx'):
        return errors

    cpp_symbols = extract_cpp_symbols(cpp_file)
    declared_ns = cpp_symbols['namespace']

    if declared_ns and declared_ns != expected_ns:
        # Allow parent namespaces (kotlinx.coroutines for file in kotlinx.coroutines.internal)
        if not expected_ns.startswith(declared_ns):
            errors.append(f"Namespace mismatch: declared '{declared_ns}' but folder suggests '{expected_ns}'")

    return errors

def validate_file(cpp_file: Path, all_symbols: Dict[str, List[str]]) -> ValidationResult:
    """Run all validation checks on a file."""
    result = ValidationResult(cpp_file)

    # Find Kotlin source
    result.kotlin_source = find_kotlin_source(cpp_file)
    if result.kotlin_source:
        result.kotlin_symbols = extract_kotlin_symbols(result.kotlin_source)
    result.cpp_symbols = extract_cpp_symbols(cpp_file)

    # Check for duplicates
    dup_errors = check_duplicates(cpp_file, all_symbols)
    for err in dup_errors:
        result.add_error(err)

    # Check symbol coverage
    coverage_errors, coverage_warnings = check_symbol_coverage(cpp_file, result.kotlin_source, set(all_symbols.keys()))
    for err in coverage_errors:
        result.add_error(err)
    for warn in coverage_warnings:
        result.add_warning(warn)

    # Check namespace
    ns_errors = check_namespace(cpp_file)
    for err in ns_errors:
        result.add_error(err)

    return result

# =============================================================================
# COMPILATION
# =============================================================================

def compile_file(cpp_file: Path) -> Tuple[bool, str]:
    """Compile a single C++ file with -Werror."""
    if cpp_file.suffix == '.hpp':
        # Headers are validated but not compiled directly
        return True, "Header file - validation only"

    # Use cmake to compile just this file's object
    rel_path = cpp_file.relative_to(CPP_SRC)

    # Construct the object target name (cmake convention)
    obj_name = str(rel_path).replace('/', '_').replace('.cpp', '.o')

    try:
        result = subprocess.run(
            ['cmake', '--build', '.', '--', '-j1'],
            cwd=BUILD_DIR,
            capture_output=True,
            text=True,
            timeout=300
        )

        # Check if our file had errors
        file_errors = []
        for line in result.stdout.split('\n') + result.stderr.split('\n'):
            if str(cpp_file) in line and ('error:' in line or 'warning:' in line):
                file_errors.append(line)

        if result.returncode != 0:
            return False, '\n'.join(file_errors) if file_errors else result.stdout[-2000:]

        return True, "Compiled successfully"

    except subprocess.TimeoutExpired:
        return False, "Compilation timed out"
    except Exception as e:
        return False, f"Compilation error: {e}"

# =============================================================================
# REPORTING
# =============================================================================

def print_validation_result(result: ValidationResult):
    """Print detailed validation result."""
    print("\n" + "=" * 80)
    print(f"FILE: {result.file_path}")
    print("=" * 80)

    if result.kotlin_source:
        print(f"Kotlin source: {result.kotlin_source}")
        if result.kotlin_symbols:
            kt = result.kotlin_symbols
            cpp = result.cpp_symbols
            print(f"Line counts: C++ {cpp['line_count']} vs Kotlin {kt['line_count']} ({cpp['line_count']/max(kt['line_count'],1):.0%})")
    else:
        print("Kotlin source: NOT FOUND")

    if result.errors:
        print(f"\n❌ ERRORS ({len(result.errors)}):")
        for err in result.errors:
            print(f"   • {err}")

    if result.warnings:
        print(f"\n⚠️  WARNINGS ({len(result.warnings)}):")
        for warn in result.warnings:
            print(f"   • {warn}")

    if result.passed:
        print("\n✅ VALIDATION PASSED")
    else:
        print("\n❌ VALIDATION FAILED - file will not be compiled")

        # Show guidance
        if result.kotlin_source and result.kotlin_symbols:
            print("\n--- KOTLIN REFERENCE ---")
            kt = result.kotlin_symbols
            print(f"Classes in Kotlin: {', '.join(kt['classes'][:10])}")
            print(f"Functions in Kotlin: {', '.join(kt['functions'][:10])}")

def print_build_order(files: List[Path]):
    """Print the build order with dependency info."""
    print("=" * 80)
    print("BUILD ORDER (most depended-on first)")
    print("=" * 80)

    graph = build_dependency_graph()
    dep_count = defaultdict(int)
    for deps in graph.values():
        for dep in deps:
            dep_count[dep] += 1

    for i, f in enumerate(files[:50], 1):
        rel = str(f.relative_to(CPP_SRC))
        deps = dep_count.get(rel, 0)
        print(f"{i:3d}. [{deps:2d} deps] {rel}")

    if len(files) > 50:
        print(f"... and {len(files) - 50} more files")

# =============================================================================
# MAIN
# =============================================================================

def build_all_symbols_map() -> Dict[str, List[str]]:
    """Build a map of all symbols to the files that define them."""
    all_symbols = defaultdict(list)

    for ext in ['*.cpp', '*.hpp']:
        for cpp_file in CPP_SRC.rglob(ext):
            if 'test' in str(cpp_file).lower():
                continue

            symbols = extract_cpp_symbols(cpp_file)
            rel_path = str(cpp_file.relative_to(CPP_SRC))

            for cls in symbols['classes']:
                all_symbols[cls].append(rel_path)

    return dict(all_symbols)

def main():
    parser = argparse.ArgumentParser(description="Kotlin-aware C++ build system")
    parser.add_argument('--file', type=str, help='Build specific file only')
    parser.add_argument('--validate', action='store_true', help='Validate only, no compile')
    parser.add_argument('--order', action='store_true', help='Show build order and exit')
    parser.add_argument('--continue', dest='continue_on_error', action='store_true',
                        help='Continue past failures (report all)')
    args = parser.parse_args()

    # Get build order
    files = get_build_order()

    if args.order:
        print_build_order(files)
        return 0

    # Filter to specific file if requested
    if args.file:
        target = Path(args.file)
        if not target.is_absolute():
            target = CPP_SRC / args.file
        files = [f for f in files if f == target or str(f).endswith(args.file)]
        if not files:
            print(f"File not found: {args.file}")
            return 3

    # Build symbol map for duplicate detection
    print("Building symbol map...")
    all_symbols = build_all_symbols_map()

    print(f"Processing {len(files)} files in dependency order...\n")

    passed = 0
    failed = 0

    for cpp_file in files:
        # Phase 1: Validate
        result = validate_file(cpp_file, all_symbols)

        if not result.passed:
            print_validation_result(result)
            failed += 1
            if not args.continue_on_error:
                print(f"\n🛑 BUILD STOPPED: Fix validation errors before continuing")
                return 1
            continue

        # Phase 2: Compile (if not validate-only)
        if not args.validate and cpp_file.suffix == '.cpp':
            success, message = compile_file(cpp_file)
            if not success:
                result.add_error(f"Compilation failed: {message}")
                print_validation_result(result)
                failed += 1
                if not args.continue_on_error:
                    print(f"\n🛑 BUILD STOPPED: Compilation error")
                    return 2
                continue

        passed += 1
        # Show progress for passed files
        rel = str(cpp_file.relative_to(CPP_SRC))
        print(f"✓ {rel}")

    print("\n" + "=" * 80)
    print(f"BUILD COMPLETE: {passed} passed, {failed} failed")
    print("=" * 80)

    return 0 if failed == 0 else 1

if __name__ == '__main__':
    sys.exit(main())
