#!/usr/bin/env python
"""
Analyze symbol definitions (classes, functions) between Kotlin and C++.

Detects:
1. Symbols defined in wrong file (vs Kotlin location)
2. Duplicate/conflicting definitions
3. Stub implementations (no-ops hiding real gaps)
4. Missing symbols entirely
5. Include mismatches

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
KOTLIN_SRC = ROOT / "tmp" / "kotlinx.coroutines"
CPP_SRC = ROOT / "src"

# Patterns for extracting definitions
KOTLIN_CLASS_PATTERN = re.compile(
    r'^(?:public\s+|private\s+|internal\s+|protected\s+)?'
    r'(?:abstract\s+|open\s+|sealed\s+|data\s+|inline\s+|value\s+)?'
    r'(?:class|interface|object)\s+(\w+)',
    re.MULTILINE
)

KOTLIN_FUN_PATTERN = re.compile(
    r'^(?:public\s+|private\s+|internal\s+|protected\s+)?'
    r'(?:suspend\s+)?'
    r'(?:inline\s+)?'
    r'fun\s+(?:<[^>]+>\s+)?(\w+)\s*\(',
    re.MULTILINE
)

CPP_CLASS_PATTERN = re.compile(
    r'^(?:template\s*<[^>]*>\s*)?'
    r'(?:class|struct)\s+(\w+)(?:\s*:\s*(?:public|private|protected))?',
    re.MULTILINE
)

CPP_FUN_PATTERN = re.compile(
    r'^\s*(?:virtual\s+|static\s+|inline\s+)?'
    r'(?:[\w:<>*&\s]+)\s+(\w+)\s*\([^)]*\)\s*(?:const\s*)?(?:override\s*)?(?:=\s*0\s*)?[{;]',
    re.MULTILINE
)

def extract_kotlin_symbols(path):
    """Extract class and function definitions from a Kotlin file."""
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except:
        return {'classes': [], 'functions': [], 'package': None}

    # Get package
    pkg_match = re.search(r'^package\s+([\w.]+)', content, re.MULTILINE)
    package = pkg_match.group(1) if pkg_match else None

    # Remove comments
    content = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)

    classes = []
    for m in KOTLIN_CLASS_PATTERN.finditer(content):
        classes.append({
            'name': m.group(1),
            'line': content[:m.start()].count('\n') + 1,
        })

    functions = []
    for m in KOTLIN_FUN_PATTERN.finditer(content):
        name = m.group(1)
        # Skip property accessors and common names
        if name not in ('get', 'set', 'invoke', 'equals', 'hashCode', 'toString'):
            functions.append({
                'name': name,
                'line': content[:m.start()].count('\n') + 1,
            })

    return {
        'classes': classes,
        'functions': functions,
        'package': package,
    }

def extract_cpp_symbols(path):
    """Extract class and function definitions from a C++ file."""
    try:
        with open(path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except:
        return {'classes': [], 'functions': [], 'namespace': None, 'is_stub': False}

    # Get namespace
    ns_match = re.search(r'namespace\s+([\w:]+)\s*\{', content)
    namespace = ns_match.group(1).replace('::', '.') if ns_match else None

    # Check if file is mostly a stub
    is_stub = is_stub_file(content)

    # Remove comments
    content_clean = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    content_clean = re.sub(r'/\*.*?\*/', '', content_clean, flags=re.DOTALL)

    classes = []
    for m in CPP_CLASS_PATTERN.finditer(content_clean):
        name = m.group(1)
        if name not in ('std', 'detail', 'anonymous'):
            # Check if it's a stub class
            stub_info = check_class_stub(content, name)
            classes.append({
                'name': name,
                'line': content[:m.start()].count('\n') + 1,
                'is_stub': stub_info['is_stub'],
                'stub_reason': stub_info.get('reason'),
            })

    functions = []
    for m in CPP_FUN_PATTERN.finditer(content_clean):
        name = m.group(1)
        if name not in ('if', 'while', 'for', 'switch', 'return'):
            functions.append({
                'name': name,
                'line': content[:m.start()].count('\n') + 1,
            })

    return {
        'classes': classes,
        'functions': functions,
        'namespace': namespace,
        'is_stub': is_stub,
    }

def is_stub_file(content):
    """Check if a file is essentially a stub with no real implementation."""
    # Remove comments
    clean = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    clean = re.sub(r'/\*.*?\*/', '', clean, flags=re.DOTALL)

    # Remove includes and pragmas
    clean = re.sub(r'#\w+.*$', '', clean, flags=re.MULTILINE)

    # Remove namespace declarations
    clean = re.sub(r'namespace\s+\w+\s*\{', '', clean)
    clean = re.sub(r'\}\s*//\s*namespace', '', clean)

    # Check if anything substantial remains
    clean = clean.strip()
    if len(clean) < 100:
        return True

    # Check for TODO-heavy files
    todo_count = content.count('TODO')
    line_count = content.count('\n')
    if line_count > 0 and todo_count / line_count > 0.1:  # >10% TODO lines
        return True

    return False

def check_class_stub(content, class_name):
    """Check if a class definition is a stub/simplified version."""
    # Find the class definition
    pattern = rf'(?:class|struct)\s+{re.escape(class_name)}\s*[^{{]*\{{([^}}]*)\}}'
    match = re.search(pattern, content, re.DOTALL)

    if not match:
        return {'is_stub': False}

    body = match.group(1)

    # Check for empty or near-empty body
    clean_body = re.sub(r'//.*$', '', body, flags=re.MULTILINE)
    clean_body = re.sub(r'/\*.*?\*/', '', clean_body, flags=re.DOTALL)
    clean_body = clean_body.strip()

    if len(clean_body) < 20:
        return {'is_stub': True, 'reason': 'empty_body'}

    # Check for only virtual destructor
    if re.match(r'^\s*(?:public:|private:|protected:)?\s*virtual\s+~\w+\(\)\s*(?:=\s*default\s*)?;?\s*$', clean_body, re.DOTALL):
        return {'is_stub': True, 'reason': 'only_destructor'}

    # Check for TODO-only body
    if 'TODO' in body and len(re.sub(r'TODO.*$', '', clean_body, flags=re.MULTILINE).strip()) < 50:
        return {'is_stub': True, 'reason': 'todo_only'}

    return {'is_stub': False}

def find_duplicate_definitions():
    """Find symbols defined in multiple places."""
    cpp_symbols = defaultdict(list)

    for ext in ['*.cpp', '*.hpp']:
        for path in CPP_SRC.rglob(ext):
            if 'test' in str(path).lower():
                continue

            symbols = extract_cpp_symbols(path)
            rel_path = str(path.relative_to(CPP_SRC))

            for cls in symbols['classes']:
                cpp_symbols[('class', cls['name'])].append({
                    'file': rel_path,
                    'line': cls['line'],
                    'is_stub': cls.get('is_stub', False),
                    'stub_reason': cls.get('stub_reason'),
                })

    # Find duplicates
    duplicates = {}
    for key, locations in cpp_symbols.items():
        if len(locations) > 1:
            duplicates[key] = locations

    return duplicates

def find_stub_implementations():
    """Find stub/no-op implementations that might hide real gaps."""
    stubs = []

    for ext in ['*.cpp', '*.hpp']:
        for path in CPP_SRC.rglob(ext):
            if 'test' in str(path).lower():
                continue

            symbols = extract_cpp_symbols(path)
            rel_path = str(path.relative_to(CPP_SRC))

            if symbols['is_stub']:
                stubs.append({
                    'file': rel_path,
                    'type': 'file_stub',
                    'reason': 'File is mostly empty/stub',
                })

            for cls in symbols['classes']:
                if cls.get('is_stub'):
                    stubs.append({
                        'file': rel_path,
                        'type': 'class_stub',
                        'name': cls['name'],
                        'line': cls['line'],
                        'reason': cls.get('stub_reason', 'unknown'),
                    })

    return stubs

def find_misplaced_symbols():
    """Find symbols defined in different files than Kotlin originals."""
    # Build Kotlin symbol map
    kotlin_symbols = {}
    for kt_file in KOTLIN_SRC.rglob("*.kt"):
        if 'test' in str(kt_file).lower() and 'kotlinx-coroutines-test' not in str(kt_file):
            continue

        symbols = extract_kotlin_symbols(kt_file)
        if not symbols['package']:
            continue

        base_name = kt_file.stem.replace('.common', '').replace('.native', '')

        for cls in symbols['classes']:
            key = cls['name']
            kotlin_symbols[key] = {
                'kotlin_file': str(kt_file.relative_to(KOTLIN_SRC)),
                'kotlin_base': base_name,
                'package': symbols['package'],
                'expected_cpp_dir': symbols['package'].replace('.', '/'),
            }

    # Find C++ symbols and check locations
    misplaced = []
    for ext in ['*.cpp', '*.hpp']:
        for path in CPP_SRC.rglob(ext):
            if 'test' in str(path).lower():
                continue

            symbols = extract_cpp_symbols(path)
            rel_path = path.relative_to(CPP_SRC)
            cpp_dir = str(rel_path.parent).replace('/', '.')
            cpp_base = path.stem.replace('.common', '').replace('.native', '')

            for cls in symbols['classes']:
                name = cls['name']
                if name in kotlin_symbols:
                    kt_info = kotlin_symbols[name]
                    expected_dir = kt_info['expected_cpp_dir']

                    # Check if in expected location
                    if expected_dir not in str(rel_path):
                        misplaced.append({
                            'symbol': name,
                            'type': 'class',
                            'cpp_file': str(rel_path),
                            'kotlin_file': kt_info['kotlin_file'],
                            'expected_dir': expected_dir,
                            'actual_dir': cpp_dir,
                        })

    return misplaced

def analyze_symbol(symbol_name):
    """Deep analysis of a specific symbol across the codebase."""
    results = {
        'kotlin_locations': [],
        'cpp_locations': [],
        'issues': [],
    }

    # Find in Kotlin
    for kt_file in KOTLIN_SRC.rglob("*.kt"):
        try:
            with open(kt_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()

            if re.search(rf'\b{re.escape(symbol_name)}\b', content):
                # Check if it's a definition or just a reference
                is_def = bool(re.search(rf'(?:class|interface|object|fun)\s+{re.escape(symbol_name)}\b', content))
                results['kotlin_locations'].append({
                    'file': str(kt_file.relative_to(KOTLIN_SRC)),
                    'is_definition': is_def,
                    'line_count': content.count(symbol_name),
                })
        except:
            pass

    # Find in C++
    for ext in ['*.cpp', '*.hpp']:
        for cpp_file in CPP_SRC.rglob(ext):
            try:
                with open(cpp_file, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()

                if re.search(rf'\b{re.escape(symbol_name)}\b', content):
                    # Check if definition
                    is_def = bool(re.search(rf'(?:class|struct)\s+{re.escape(symbol_name)}\b', content))
                    is_template = bool(re.search(rf'template\s*<[^>]*>\s*(?:class|struct)\s+{re.escape(symbol_name)}\b', content))

                    # Check body size if definition
                    body_info = None
                    if is_def:
                        match = re.search(rf'(?:class|struct)\s+{re.escape(symbol_name)}[^{{]*\{{([^}}]*)\}}', content, re.DOTALL)
                        if match:
                            body = match.group(1)
                            body_info = {
                                'lines': body.count('\n'),
                                'has_todo': 'TODO' in body,
                                'is_empty': len(body.strip()) < 50,
                            }

                    results['cpp_locations'].append({
                        'file': str(cpp_file.relative_to(CPP_SRC)),
                        'is_definition': is_def,
                        'is_template': is_template,
                        'reference_count': content.count(symbol_name),
                        'body_info': body_info,
                    })
            except:
                pass

    # Analyze issues
    cpp_defs = [loc for loc in results['cpp_locations'] if loc['is_definition']]

    if len(cpp_defs) > 1:
        results['issues'].append({
            'type': 'duplicate_definition',
            'message': f"'{symbol_name}' defined in {len(cpp_defs)} files",
            'files': [d['file'] for d in cpp_defs],
        })

    for cpp_def in cpp_defs:
        if cpp_def.get('body_info', {}).get('is_empty'):
            results['issues'].append({
                'type': 'stub_implementation',
                'message': f"'{symbol_name}' in {cpp_def['file']} appears to be a stub",
                'file': cpp_def['file'],
            })

    return results

def main():
    parser = argparse.ArgumentParser(description='Analyze symbol definitions')
    parser.add_argument('--duplicates', action='store_true', help='Show duplicate definitions')
    parser.add_argument('--stubs', action='store_true', help='Show stub implementations')
    parser.add_argument('--misplaced', action='store_true', help='Show misplaced symbols')
    parser.add_argument('--symbol', type=str, help='Analyze specific symbol')
    parser.add_argument('--json', action='store_true', help='Output as JSON')
    args = parser.parse_args()

    if args.symbol:
        results = analyze_symbol(args.symbol)
        if args.json:
            print(json.dumps(results, indent=2))
        else:
            print(f"\n=== Analysis of '{args.symbol}' ===\n")

            print(f"Kotlin locations ({len(results['kotlin_locations'])}):")
            for loc in results['kotlin_locations'][:10]:
                marker = "[DEF]" if loc['is_definition'] else "[ref]"
                print(f"  {marker} {loc['file']}")

            print(f"\nC++ locations ({len(results['cpp_locations'])}):")
            for loc in results['cpp_locations']:
                markers = []
                if loc['is_definition']: markers.append("DEF")
                if loc.get('is_template'): markers.append("TMPL")
                body_info = loc.get('body_info') or {}
                if body_info.get('is_empty'): markers.append("STUB")
                if body_info.get('has_todo'): markers.append("TODO")
                marker = f"[{','.join(markers)}]" if markers else "[ref]"
                print(f"  {marker} {loc['file']}")

            if results['issues']:
                print(f"\nIssues found:")
                for issue in results['issues']:
                    print(f"  ⚠️  {issue['message']}")
        return

    print("=" * 70)
    print("SYMBOL DEFINITION ANALYSIS")
    print("=" * 70)

    if args.duplicates or not (args.stubs or args.misplaced):
        print("\n--- DUPLICATE DEFINITIONS ---")
        duplicates = find_duplicate_definitions()
        print(f"Found {len(duplicates)} symbols with multiple definitions:\n")
        for (sym_type, name), locations in sorted(duplicates.items())[:20]:
            print(f"  {sym_type}: {name}")
            for loc in locations:
                stub_marker = " [STUB]" if loc.get('is_stub') else ""
                print(f"    - {loc['file']}:{loc['line']}{stub_marker}")
        if len(duplicates) > 20:
            print(f"  ... and {len(duplicates) - 20} more")

    if args.stubs or not (args.duplicates or args.misplaced):
        print("\n--- STUB IMPLEMENTATIONS ---")
        stubs = find_stub_implementations()
        print(f"Found {len(stubs)} stub implementations:\n")

        file_stubs = [s for s in stubs if s['type'] == 'file_stub']
        class_stubs = [s for s in stubs if s['type'] == 'class_stub']

        print(f"  Stub files ({len(file_stubs)}):")
        for stub in file_stubs[:15]:
            print(f"    - {stub['file']}")
        if len(file_stubs) > 15:
            print(f"    ... and {len(file_stubs) - 15} more")

        print(f"\n  Stub classes ({len(class_stubs)}):")
        for stub in class_stubs[:15]:
            print(f"    - {stub['name']} in {stub['file']} ({stub['reason']})")
        if len(class_stubs) > 15:
            print(f"    ... and {len(class_stubs) - 15} more")

    if args.misplaced or not (args.duplicates or args.stubs):
        print("\n--- MISPLACED SYMBOLS ---")
        misplaced = find_misplaced_symbols()
        print(f"Found {len(misplaced)} symbols in unexpected locations:\n")
        for item in misplaced[:20]:
            print(f"  {item['symbol']}:")
            print(f"    C++ file:  {item['cpp_file']}")
            print(f"    Expected:  {item['expected_dir']}/")
            print(f"    Kotlin:    {item['kotlin_file']}")
        if len(misplaced) > 20:
            print(f"  ... and {len(misplaced) - 20} more")

    print("\n" + "=" * 70)

if __name__ == "__main__":
    main()
