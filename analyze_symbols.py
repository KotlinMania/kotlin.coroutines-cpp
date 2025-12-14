#!/usr/bin/env python
"""
Analyze symbol definitions (classes, functions) between Kotlin and C++.

Detects:
1. Symbols defined in wrong file (vs Kotlin location)
2. Duplicate/conflicting definitions (real definitions, not forward declarations)
3. Stub implementations (no-ops hiding real gaps)
4. Missing symbols entirely
5. Include mismatches

Results are ORDERED BY DEPENDENCY COUNT - most-imported Kotlin files first.

Usage:
    python analyze_symbols.py                    # Full analysis
    python analyze_symbols.py --duplicates      # Show duplicate definitions
    python analyze_symbols.py --stubs           # Show stub/no-op implementations
    python analyze_symbols.py --misplaced       # Show symbols in wrong files
    python analyze_symbols.py --symbol NAME     # Analyze specific symbol
"""

import os
import re
import sys
import json
import argparse
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).parent
KOTLIN_SRC = ROOT / "tmp" / "kotlinx.coroutines" / "kotlinx-coroutines-core"
CPP_SRC = ROOT / "src"

# =============================================================================
# DEPENDENCY ANALYSIS (from all_reports.py)
# =============================================================================

def build_kotlin_dependency_graph():
    """
    Build a dependency graph of Kotlin files and return files sorted by
    how many other files depend on them (most depended-on first).
    """
    kt_files = []
    class_to_file = {}
    file_pkg_map = {}
    pkg_to_files = defaultdict(list)
    file_contents = {}
    file_classes = defaultdict(list)

    for f in KOTLIN_SRC.rglob("*.kt"):
        if 'test' in str(f).lower():
            continue
        try:
            content = f.read_text(errors='ignore')
            pkg_m = re.search(r'^package\s+([\w.]+)', content, re.MULTILINE)
            if not pkg_m:
                continue
            pkg = pkg_m.group(1)

            kt_files.append(f)
            file_pkg_map[f] = pkg
            pkg_to_files[pkg].append(f)
            file_contents[f] = content

            # Extract classes
            for m in re.finditer(r'(?:class|interface|object)\s+(\w+)', content):
                cls_name = m.group(1)
                file_classes[f].append(cls_name)
                class_to_file[f"{pkg}.{cls_name}"] = f

        except Exception:
            pass

    # Build Graph
    graph = defaultdict(set)
    for f in kt_files:
        content = file_contents[f]
        pkg = file_pkg_map[f]

        # Explicit imports
        imports = re.findall(r'^import\s+([\w.]+)', content, re.MULTILINE)
        for imp in imports:
            if imp in class_to_file:
                dep = class_to_file[imp]
                if dep != f:
                    graph[f].add(dep)
            elif imp.endswith('.*'):
                target_pkg = imp[:-2]
                for neighbor in pkg_to_files.get(target_pkg, []):
                    if neighbor != f:
                        for cls in file_classes[neighbor]:
                            if re.search(rf'\b{cls}\b', content):
                                graph[f].add(neighbor)
                                break

        # Implicit same-package
        for neighbor in pkg_to_files[pkg]:
            if neighbor == f:
                continue
            for cls in file_classes[neighbor]:
                if re.search(rf'\b{cls}\b', content):
                    graph[f].add(neighbor)
                    break

    # Count how many files depend on each file
    usage_count = defaultdict(int)
    for consumer, providers in graph.items():
        for provider in providers:
            usage_count[provider] += 1

    # Sort by dependency count (most depended-on first)
    sorted_kt = sorted(kt_files, key=lambda f: (-usage_count[f], str(f)))

    return sorted_kt, usage_count, file_classes, file_pkg_map


# =============================================================================
# C++ SYMBOL EXTRACTION (improved to skip forward declarations)
# =============================================================================

# Pattern for REAL class/struct definitions (must have { body)
# This excludes forward declarations like "class Foo;" or "template<T> class Foo;"
CPP_CLASS_DEF_PATTERN = re.compile(
    r'^(?:template\s*<[^>]*>\s*)?'
    r'(class|struct)\s+(\w+)'
    r'(?:\s*:\s*[^{]+)?'  # optional inheritance
    r'\s*\{',  # MUST have opening brace
    re.MULTILINE
)

# Pattern for forward declarations (no body)
CPP_FORWARD_DECL_PATTERN = re.compile(
    r'^(?:template\s*<[^>]*>\s*)?'
    r'(class|struct)\s+(\w+)\s*;',
    re.MULTILINE
)


def extract_cpp_class_definitions(path):
    """
    Extract REAL class definitions from a C++ file.
    Excludes forward declarations.
    """
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except Exception:
        return []

    # Remove comments first
    content_clean = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    content_clean = re.sub(r'/\*.*?\*/', '', content_clean, flags=re.DOTALL)

    classes = []
    seen = set()  # Track (name, line) to avoid duplicates from nested classes

    for m in CPP_CLASS_DEF_PATTERN.finditer(content_clean):
        kind = m.group(1)  # class or struct
        name = m.group(2)

        if name in ('std', 'detail', 'anonymous', '__'):
            continue

        line = content_clean[:m.start()].count('\n') + 1
        key = (name, line)
        if key in seen:
            continue
        seen.add(key)

        # Check if this is a real definition with substantial body
        # Find the matching closing brace
        start = m.end() - 1  # position of {
        body = extract_class_body(content_clean, start)

        is_stub = False
        stub_reason = None

        if body is not None:
            clean_body = body.strip()
            # Check for empty or near-empty body
            if len(clean_body) < 30:
                is_stub = True
                stub_reason = 'empty_body'
            # Check for only destructor
            elif re.match(r'^\s*(?:public:|private:|protected:)?\s*(?:virtual\s+)?~\w+\([^)]*\)\s*(?:=\s*default\s*)?;?\s*$', clean_body, re.DOTALL):
                is_stub = True
                stub_reason = 'only_destructor'
            # Check for pure virtual only (interface that's just = 0 declarations)
            elif clean_body.count('= 0;') > 0 and len(re.sub(r'virtual[^;]+= 0;', '', clean_body).strip()) < 50:
                # This is an interface, not a stub
                pass

        classes.append({
            'name': name,
            'kind': kind,
            'line': line,
            'is_stub': is_stub,
            'stub_reason': stub_reason,
        })

    return classes


def extract_class_body(content, start_pos):
    """Extract the body of a class starting at the opening brace."""
    if start_pos >= len(content) or content[start_pos] != '{':
        return None

    depth = 1
    idx = start_pos + 1
    while idx < len(content) and depth > 0:
        if content[idx] == '{':
            depth += 1
        elif content[idx] == '}':
            depth -= 1
        idx += 1

    if depth != 0:
        return None

    return content[start_pos + 1:idx - 1]


def is_stub_file(path):
    """Check if a file is essentially a stub with no real implementation."""
    try:
        content = path.read_text(errors='ignore')
    except Exception:
        return False

    # Remove comments
    clean = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    clean = re.sub(r'/\*.*?\*/', '', clean, flags=re.DOTALL)

    # Remove includes and pragmas
    clean = re.sub(r'#\w+.*$', '', clean, flags=re.MULTILINE)

    # Remove namespace declarations
    clean = re.sub(r'namespace\s+[\w:]+\s*\{', '', clean)
    clean = re.sub(r'\}\s*//\s*namespace', '', clean)

    clean = clean.strip()
    if len(clean) < 100:
        return True

    return False


# =============================================================================
# KOTLIN TO C++ MAPPING
# =============================================================================

def map_kotlin_to_cpp(kt_file):
    """Find C++ files that correspond to a Kotlin file."""
    base = kt_file.stem.replace('.common', '').replace('.native', '')
    cpp_matches = []

    # Direct name match
    for cpp in CPP_SRC.rglob(f"{base}.*"):
        if cpp.suffix in {'.hpp', '.cpp', '.h'}:
            cpp_matches.append(cpp)

    # Snake case fallback
    if not cpp_matches:
        snake = re.sub(r'(?<!^)(?=[A-Z])', '_', base).lower()
        for cpp in CPP_SRC.rglob(f"{snake}.*"):
            if cpp.suffix in {'.hpp', '.cpp', '.h'}:
                cpp_matches.append(cpp)

    return cpp_matches


# =============================================================================
# ANALYSIS FUNCTIONS
# =============================================================================

def find_duplicate_definitions(sorted_kt, usage_count):
    """
    Find symbols defined in multiple C++ files.
    Returns results ordered by Kotlin dependency count.
    """
    # First, collect all C++ class definitions
    cpp_symbols = defaultdict(list)

    for path in CPP_SRC.rglob('*.hpp'):
        if 'test' in str(path).lower():
            continue

        classes = extract_cpp_class_definitions(path)
        rel_path = str(path.relative_to(CPP_SRC))

        for cls in classes:
            cpp_symbols[cls['name']].append({
                'file': rel_path,
                'line': cls['line'],
                'kind': cls['kind'],
                'is_stub': cls.get('is_stub', False),
                'stub_reason': cls.get('stub_reason'),
            })

    # Also check .cpp files for definitions (less common but possible)
    for path in CPP_SRC.rglob('*.cpp'):
        if 'test' in str(path).lower():
            continue

        classes = extract_cpp_class_definitions(path)
        rel_path = str(path.relative_to(CPP_SRC))

        for cls in classes:
            cpp_symbols[cls['name']].append({
                'file': rel_path,
                'line': cls['line'],
                'kind': cls['kind'],
                'is_stub': cls.get('is_stub', False),
                'stub_reason': cls.get('stub_reason'),
            })

    # Find actual duplicates (same class defined in multiple files)
    duplicates = {}
    for name, locations in cpp_symbols.items():
        # Group by file to find cross-file duplicates
        files = set(loc['file'] for loc in locations)
        if len(files) > 1:
            duplicates[name] = locations

    # Order by Kotlin dependency: find which Kotlin file defines each symbol
    # and use its usage_count for ordering
    kt_symbol_to_file = {}
    for kt_file in sorted_kt:
        try:
            content = kt_file.read_text(errors='ignore')
            for m in re.finditer(r'(?:class|interface|object)\s+(\w+)', content):
                cls_name = m.group(1)
                if cls_name not in kt_symbol_to_file:
                    kt_symbol_to_file[cls_name] = kt_file
        except Exception:
            pass

    def get_priority(name):
        kt_file = kt_symbol_to_file.get(name)
        if kt_file:
            return -usage_count.get(kt_file, 0)
        return 0

    sorted_duplicates = sorted(duplicates.items(), key=lambda x: (get_priority(x[0]), x[0]))

    return sorted_duplicates


def find_stub_implementations(sorted_kt, usage_count):
    """Find stub files and classes, ordered by dependency."""
    stubs = []

    # Check stub files
    for path in CPP_SRC.rglob('*.cpp'):
        if 'test' in str(path).lower():
            continue
        if is_stub_file(path):
            stubs.append({
                'file': str(path.relative_to(CPP_SRC)),
                'type': 'file_stub',
                'name': path.stem,
            })

    # Check stub classes in headers
    for path in CPP_SRC.rglob('*.hpp'):
        if 'test' in str(path).lower():
            continue
        classes = extract_cpp_class_definitions(path)
        for cls in classes:
            if cls.get('is_stub'):
                stubs.append({
                    'file': str(path.relative_to(CPP_SRC)),
                    'type': 'class_stub',
                    'name': cls['name'],
                    'line': cls['line'],
                    'reason': cls.get('stub_reason', 'unknown'),
                })

    # Build Kotlin symbol -> dependency count map
    kt_symbol_deps = {}
    for kt_file in sorted_kt:
        try:
            content = kt_file.read_text(errors='ignore')
            for m in re.finditer(r'(?:class|interface|object)\s+(\w+)', content):
                cls_name = m.group(1)
                if cls_name not in kt_symbol_deps:
                    kt_symbol_deps[cls_name] = usage_count.get(kt_file, 0)
        except Exception:
            pass

    # Also map file stems to dependency counts
    kt_file_deps = {kt.stem.replace('.common', '').replace('.native', ''): usage_count.get(kt, 0)
                    for kt in sorted_kt}

    def get_priority(stub):
        name = stub['name']
        # Check class name first
        if name in kt_symbol_deps:
            return -kt_symbol_deps[name]
        # Then file stem
        file_stem = Path(stub['file']).stem.replace('.common', '').replace('.native', '')
        if file_stem in kt_file_deps:
            return -kt_file_deps[file_stem]
        return 0

    stubs.sort(key=lambda s: (get_priority(s), s['file']))

    return stubs


def analyze_symbol(symbol_name, sorted_kt, usage_count):
    """Deep analysis of a specific symbol across the codebase."""
    results = {
        'kotlin_locations': [],
        'cpp_locations': [],
        'issues': [],
        'dependency_rank': None,
    }

    # Find in Kotlin
    for kt_file in sorted_kt:
        try:
            content = kt_file.read_text(errors='ignore')

            if re.search(rf'\b{re.escape(symbol_name)}\b', content):
                is_def = bool(re.search(rf'(?:class|interface|object|fun)\s+{re.escape(symbol_name)}\b', content))
                results['kotlin_locations'].append({
                    'file': str(kt_file.relative_to(KOTLIN_SRC)),
                    'is_definition': is_def,
                    'deps': usage_count.get(kt_file, 0),
                })
                if is_def and results['dependency_rank'] is None:
                    results['dependency_rank'] = usage_count.get(kt_file, 0)
        except Exception:
            pass

    # Find in C++
    for ext in ['*.cpp', '*.hpp']:
        for cpp_file in CPP_SRC.rglob(ext):
            if 'test' in str(cpp_file).lower():
                continue
            try:
                content = cpp_file.read_text(errors='ignore')

                if re.search(rf'\b{re.escape(symbol_name)}\b', content):
                    # Check if it's a real definition (with body)
                    is_def = bool(re.search(
                        rf'(?:class|struct)\s+{re.escape(symbol_name)}(?:\s*:[^{{]+)?\s*\{{',
                        content
                    ))
                    is_forward = bool(re.search(
                        rf'(?:class|struct)\s+{re.escape(symbol_name)}\s*;',
                        content
                    ))

                    results['cpp_locations'].append({
                        'file': str(cpp_file.relative_to(CPP_SRC)),
                        'is_definition': is_def,
                        'is_forward_decl': is_forward and not is_def,
                        'reference_count': content.count(symbol_name),
                    })
            except Exception:
                pass

    # Analyze issues
    cpp_defs = [loc for loc in results['cpp_locations'] if loc['is_definition']]

    if len(cpp_defs) > 1:
        # Check if they're in different files
        files = set(d['file'] for d in cpp_defs)
        if len(files) > 1:
            results['issues'].append({
                'type': 'duplicate_definition',
                'message': f"'{symbol_name}' defined in {len(files)} files: {', '.join(sorted(files))}",
            })

    return results


# =============================================================================
# MAIN
# =============================================================================

def main():
    parser = argparse.ArgumentParser(description='Analyze symbol definitions (ordered by dependency)')
    parser.add_argument('--duplicates', action='store_true', help='Show duplicate definitions')
    parser.add_argument('--stubs', action='store_true', help='Show stub implementations')
    parser.add_argument('--misplaced', action='store_true', help='Show misplaced symbols')
    parser.add_argument('--symbol', type=str, help='Analyze specific symbol')
    parser.add_argument('--json', action='store_true', help='Output as JSON')
    args = parser.parse_args()

    print("Building Kotlin dependency graph...")
    sorted_kt, usage_count, file_classes, file_pkg_map = build_kotlin_dependency_graph()
    print(f"Analyzed {len(sorted_kt)} Kotlin files\n")

    if args.symbol:
        results = analyze_symbol(args.symbol, sorted_kt, usage_count)
        if args.json:
            print(json.dumps(results, indent=2))
        else:
            print(f"\n=== Analysis of '{args.symbol}' ===")
            if results['dependency_rank'] is not None:
                print(f"Dependency rank: {results['dependency_rank']} files depend on this\n")

            print(f"Kotlin locations ({len(results['kotlin_locations'])}):")
            for loc in results['kotlin_locations'][:10]:
                marker = "[DEF]" if loc['is_definition'] else "[ref]"
                deps = f" (deps: {loc['deps']})" if loc['deps'] > 0 else ""
                print(f"  {marker} {loc['file']}{deps}")

            print(f"\nC++ locations ({len(results['cpp_locations'])}):")
            for loc in results['cpp_locations']:
                if loc['is_definition']:
                    marker = "[DEF]"
                elif loc.get('is_forward_decl'):
                    marker = "[fwd]"
                else:
                    marker = "[ref]"
                print(f"  {marker} {loc['file']}")

            if results['issues']:
                print(f"\nIssues found:")
                for issue in results['issues']:
                    print(f"  {issue['message']}")
        return

    print("=" * 70)
    print("SYMBOL DEFINITION ANALYSIS (ordered by dependency count)")
    print("=" * 70)

    if args.duplicates or not (args.stubs or args.misplaced):
        print("\n--- DUPLICATE DEFINITIONS (real definitions in multiple files) ---")
        duplicates = find_duplicate_definitions(sorted_kt, usage_count)
        print(f"Found {len(duplicates)} symbols with multiple definitions:\n")

        for name, locations in duplicates[:30]:
            # Group by file
            by_file = defaultdict(list)
            for loc in locations:
                by_file[loc['file']].append(loc)

            print(f"  class: {name}")
            for file, locs in sorted(by_file.items()):
                stub_marker = " [STUB]" if any(l.get('is_stub') for l in locs) else ""
                lines = ", ".join(str(l['line']) for l in locs)
                print(f"    - {file}:{lines}{stub_marker}")

        if len(duplicates) > 30:
            print(f"\n  ... and {len(duplicates) - 30} more")

    if args.stubs or not (args.duplicates or args.misplaced):
        print("\n--- STUB IMPLEMENTATIONS (ordered by dependency) ---")
        stubs = find_stub_implementations(sorted_kt, usage_count)

        file_stubs = [s for s in stubs if s['type'] == 'file_stub']
        class_stubs = [s for s in stubs if s['type'] == 'class_stub']

        print(f"\nStub files ({len(file_stubs)}):")
        for stub in file_stubs[:20]:
            print(f"    - {stub['file']}")
        if len(file_stubs) > 20:
            print(f"    ... and {len(file_stubs) - 20} more")

        print(f"\nStub classes ({len(class_stubs)}):")
        for stub in class_stubs[:20]:
            print(f"    - {stub['name']} in {stub['file']} ({stub['reason']})")
        if len(class_stubs) > 20:
            print(f"    ... and {len(class_stubs) - 20} more")

    print("\n" + "=" * 70)


if __name__ == "__main__":
    main()
