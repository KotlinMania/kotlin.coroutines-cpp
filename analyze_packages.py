#!/usr/bin/env python
"""
Analyze Kotlin packages vs C++ namespaces to find gaps and mismatches.
Also checks import/include dependencies between files.
"""

import os
import re
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).parent
KOTLIN_SRC = ROOT / "tmp" / "kotlinx.coroutines"
CPP_SRC = ROOT / "src"
CPP_INCLUDE = ROOT / "include"

def extract_kotlin_packages():
    """Extract package declarations and imports from all .kt files."""
    packages = defaultdict(list)
    kotlin_files = {}  # filepath -> {package, imports}

    for kt_file in KOTLIN_SRC.rglob("*.kt"):
        rel_path = kt_file.relative_to(KOTLIN_SRC)
        with open(kt_file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        # Extract package
        pkg_match = re.search(r'^package\s+([\w.]+)', content, re.MULTILINE)
        pkg = pkg_match.group(1) if pkg_match else None

        if pkg:
            packages[pkg].append(str(rel_path))

        # Extract imports (focus on kotlinx.coroutines imports)
        imports = []
        for import_match in re.finditer(r'^import\s+(kotlinx\.coroutines[.\w]*)', content, re.MULTILINE):
            imported = import_match.group(1)
            # Strip specific class/function names to get package
            if '.' in imported:
                parts = imported.split('.')
                # Keep up to the likely package boundary (e.g., kotlinx.coroutines.channels)
                # This is approximate - imports can be class names too
                imports.append(imported)

        kotlin_files[str(rel_path)] = {
            'package': pkg,
            'imports': imports,
            'full_path': str(kt_file)
        }

    return packages, kotlin_files

def folder_to_expected_namespace(folder_path):
    """
    Convert a folder path to the expected C++ namespace.
    Handles platform source sets (common, native, concurrent) by stripping them.

    Note: 'test' is NOT stripped - kotlinx.coroutines.test is a real package.
    Only 'src' subdirs within platform dirs are stripped.
    """
    parts = folder_path.split('.')

    # Filter out platform source set markers and normalize
    # kotlinx.coroutines.common.internal -> kotlinx.coroutines.internal
    # kotlinx.coroutines.native -> kotlinx.coroutines
    # kotlinx.coroutines.concurrent.channels -> kotlinx.coroutines.channels
    # BUT: kotlinx.coroutines.test -> kotlinx.coroutines.test (real package!)
    filtered = []
    i = 0
    while i < len(parts):
        part = parts[i]
        if part in ('common', 'native', 'concurrent'):
            # Platform marker - skip it, but check if next is 'test' or 'src'
            if i + 1 < len(parts) and parts[i + 1] == 'test':
                # native.test -> just skip native, keep test as part of path?
                # Actually native/test/ contains tests FOR native, not kotlinx.coroutines.test package
                # These should map to kotlinx.coroutines (the test files test that package)
                i += 1  # skip 'native'
                continue
            i += 1
            continue
        if part == 'src':
            # Skip 'src' directory markers
            i += 1
            continue
        filtered.append(part)
        i += 1

    return '.'.join(filtered) if filtered else folder_path

def extract_cpp_namespaces():
    """Extract namespace declarations and includes from all .cpp/.hpp files."""
    namespaces = {}

    # Check both src and include directories
    for search_root in [CPP_SRC, CPP_INCLUDE]:
        if not search_root.exists():
            continue

        for ext in ['*.cpp', '*.hpp']:
            for cpp_file in search_root.rglob(ext):
                # Skip build directories
                if 'build' in str(cpp_file):
                    continue

                rel_path = cpp_file.relative_to(search_root)
                folder_path = str(rel_path.parent).replace('/', '.')

                with open(cpp_file, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()

                # Extract namespace declarations
                declared_ns = extract_deepest_namespace(content)
                expected_ns = folder_to_expected_namespace(folder_path)

                # Extract includes (focus on kotlinx/coroutines includes)
                includes = []
                for include_match in re.finditer(r'#include\s*[<"](kotlinx/coroutines[/\w.]*)[>"]', content):
                    included = include_match.group(1)
                    includes.append(included)

                key = f"{search_root.name}/{rel_path}"
                namespaces[key] = {
                    'folder': folder_path,
                    'expected': expected_ns,
                    'declared': declared_ns,
                    'filename': cpp_file.name,
                    'full_path': str(cpp_file),
                    'includes': includes,
                    'root': search_root.name
                }

    return namespaces

def extract_deepest_namespace(content):
    """
    Extract the deepest/most specific namespace from C++ content.
    Handles both nested and C++17 style declarations.
    """
    # Remove comments and strings to avoid false matches
    # Simple removal - not perfect but good enough
    content_clean = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    content_clean = re.sub(r'/\*.*?\*/', '', content_clean, flags=re.DOTALL)

    namespaces_found = []

    # Pattern for nested namespaces: namespace X { namespace Y {
    # We need to track nesting depth
    lines = content_clean.split('\n')
    current_ns_stack = []

    for line in lines:
        # Check for namespace opening
        ns_match = re.match(r'\s*namespace\s+(\w+)\s*\{', line)
        if ns_match:
            ns_name = ns_match.group(1)
            if ns_name not in ('std', 'detail', 'anonymous'):
                current_ns_stack.append(ns_name)
                if current_ns_stack and current_ns_stack[0] == 'kotlinx':
                    namespaces_found.append('.'.join(current_ns_stack))

        # Check for C++17 style: namespace X::Y::Z {
        ns17_match = re.match(r'\s*namespace\s+([\w:]+)\s*\{', line)
        if ns17_match and '::' in ns17_match.group(1):
            ns_path = ns17_match.group(1).replace('::', '.')
            if ns_path.startswith('kotlinx'):
                namespaces_found.append(ns_path)

        # Check for closing braces (simplified - just track when we see } // namespace
        close_match = re.search(r'\}\s*//\s*namespace\s+(\w+)', line)
        if close_match and current_ns_stack:
            if current_ns_stack[-1] == close_match.group(1):
                current_ns_stack.pop()

    # Return the deepest/most specific namespace found
    if namespaces_found:
        return max(namespaces_found, key=lambda x: x.count('.'))
    return None

def analyze_cpp_namespace_consistency(cpp_data):
    """Check if C++ files have namespaces matching their folder locations."""
    print("\n" + "="*80)
    print("C++ NAMESPACE vs FOLDER CONSISTENCY CHECK")
    print("="*80)

    mismatches = []
    no_namespace = []
    correct = []

    for filepath, info in sorted(cpp_data.items()):
        folder = info['folder']
        expected = info['expected']
        declared = info['declared']

        # Skip non-kotlinx files
        if not folder.startswith('kotlinx.coroutines'):
            continue

        # Skip tools, integration, tests - these have different conventions
        if any(x in folder for x in ['.tools.', '.integration.', 'tests.']):
            continue

        if not declared:
            no_namespace.append((filepath, expected))
            continue

        # Check if declared namespace matches expected
        # Allow for the declared to be a parent of expected (e.g., kotlinx.coroutines for a file in kotlinx.coroutines.internal)
        # But flag if declared is MORE specific than expected or completely different
        if declared == expected:
            correct.append((filepath, expected, declared))
        elif expected.startswith(declared + '.'):
            # Declared is a parent namespace - might be intentional but worth noting
            mismatches.append((filepath, expected, declared, 'PARENT_NS'))
        elif declared.startswith(expected + '.'):
            # Declared is more specific than folder suggests - unusual
            mismatches.append((filepath, expected, declared, 'CHILD_NS'))
        else:
            # Completely different
            mismatches.append((filepath, expected, declared, 'DIFFERENT'))

    print(f"\n### Summary:")
    print(f"  Correct namespace:    {len(correct)}")
    print(f"  Missing namespace:    {len(no_namespace)}")
    print(f"  Mismatched namespace: {len(mismatches)}")

    if no_namespace:
        print(f"\n### Files with NO namespace declaration ({len(no_namespace)}):")
        for filepath, expected in no_namespace:
            print(f"  {filepath}")
            print(f"    Expected: {expected}")

    if mismatches:
        print(f"\n### Files with MISMATCHED namespace ({len(mismatches)}):")

        # Group by mismatch type
        by_type = defaultdict(list)
        for item in mismatches:
            by_type[item[3]].append(item[:3])

        for mtype, items in sorted(by_type.items()):
            print(f"\n  [{mtype}] ({len(items)} files):")
            for filepath, expected, declared in items:
                print(f"    {filepath}")
                print(f"      Folder->Expected: {expected}")
                print(f"      Declared:         {declared}")

    return mismatches, no_namespace, correct

def compare_packages_to_folders(kotlin_pkgs, cpp_data):
    """Compare Kotlin packages to C++ folder structure."""
    print("\n" + "="*80)
    print("KOTLIN PACKAGES vs C++ FOLDERS COMPARISON")
    print("="*80)

    # Get unique C++ folders and their expected namespaces
    cpp_folders = set()
    cpp_expected_ns = set()
    for filepath, info in cpp_data.items():
        folder = info['folder']
        expected = info['expected']
        if folder.startswith('kotlinx.coroutines'):
            cpp_folders.add(folder)
            cpp_expected_ns.add(expected)

    # Kotlin packages (filter to kotlinx.coroutines.*)
    kt_pkgs = set(p for p in kotlin_pkgs.keys() if p.startswith('kotlinx.coroutines'))

    print(f"\nKotlin packages:          {len(kt_pkgs)}")
    print(f"C++ folders:              {len(cpp_folders)}")
    print(f"C++ expected namespaces:  {len(cpp_expected_ns)}")

    # Compare Kotlin packages to C++ expected namespaces (normalized)
    kt_only = kt_pkgs - cpp_expected_ns
    cpp_only = cpp_expected_ns - kt_pkgs

    print(f"\n### Kotlin packages with NO matching C++ namespace:")
    for pkg in sorted(kt_only):
        print(f"  {pkg}")
        print(f"    Kotlin files: {len(kotlin_pkgs[pkg])}")

    print(f"\n### C++ namespaces with NO matching Kotlin package:")
    for ns in sorted(cpp_only):
        # Find which files use this namespace
        files = [f for f, info in cpp_data.items() if info['expected'] == ns]
        print(f"  {ns}")
        print(f"    C++ files: {len(files)}")

    # Show the raw folder structure too
    print(f"\n### C++ folder structure (raw):")
    for folder in sorted(cpp_folders):
        expected = folder_to_expected_namespace(folder)
        if folder != expected:
            print(f"  {folder} -> {expected}")
        else:
            print(f"  {folder}")

def list_files_by_package(kotlin_pkgs, cpp_data):
    """List files grouped by package/folder."""
    print("\n" + "="*80)
    print("FILES BY PACKAGE/FOLDER")
    print("="*80)

    # Group C++ files by folder
    cpp_by_folder = defaultdict(list)
    for filepath, info in cpp_data.items():
        folder = info['folder']
        cpp_by_folder[folder].append(info['filename'])

    # Get all unique packages/folders
    all_pkgs = set(kotlin_pkgs.keys()) | set(cpp_by_folder.keys())
    all_pkgs = sorted(p for p in all_pkgs if p.startswith('kotlinx.coroutines'))

    for pkg in all_pkgs:
        kt_files = [os.path.basename(f) for f in kotlin_pkgs.get(pkg, [])]
        cpp_files = cpp_by_folder.get(pkg, [])

        # Get base names without extension for comparison
        kt_bases = set(os.path.splitext(f)[0].replace('.common', '').replace('.kt', '') for f in kt_files)
        cpp_bases = set(os.path.splitext(f)[0].replace('.common', '').replace('.cpp', '').replace('.hpp', '') for f in cpp_files)

        kt_only = kt_bases - cpp_bases
        cpp_only = cpp_bases - kt_bases

        if kt_only or cpp_only:
            print(f"\n### {pkg}")
            if kt_only:
                print(f"  Kotlin only ({len(kt_only)}): {', '.join(sorted(kt_only)[:10])}")
                if len(kt_only) > 10:
                    print(f"    ... and {len(kt_only) - 10} more")
            if cpp_only:
                print(f"  C++ only ({len(cpp_only)}): {', '.join(sorted(cpp_only)[:10])}")
                if len(cpp_only) > 10:
                    print(f"    ... and {len(cpp_only) - 10} more")

def analyze_import_dependencies(kotlin_files, cpp_data):
    """Analyze and compare import/include dependencies."""
    print("\n" + "="*80)
    print("IMPORT/INCLUDE DEPENDENCY ANALYSIS")
    print("="*80)

    # Build a map of filename -> imports/includes for comparison
    # We need to match Kotlin files to their C++ equivalents

    # Get Kotlin file base names and their imports
    kotlin_imports = {}
    for kt_path, kt_info in kotlin_files.items():
        base_name = os.path.basename(kt_path)
        base_name = os.path.splitext(base_name)[0]
        # Strip platform qualifiers
        base_name = base_name.replace('.common', '').replace('.native', '').replace('.concurrent', '')

        if base_name not in kotlin_imports:
            kotlin_imports[base_name] = {'files': [], 'imports': set(), 'package': kt_info['package']}

        kotlin_imports[base_name]['files'].append(kt_path)
        kotlin_imports[base_name]['imports'].update(kt_info['imports'])

    # Get C++ file base names and their includes
    cpp_includes = {}
    for cpp_path, cpp_info in cpp_data.items():
        base_name = cpp_info['filename']
        base_name = os.path.splitext(base_name)[0]
        # Strip platform qualifiers
        base_name = base_name.replace('.common', '').replace('.native', '').replace('.concurrent', '')

        if base_name not in cpp_includes:
            cpp_includes[base_name] = {'files': [], 'includes': set(), 'namespace': cpp_info['declared']}

        cpp_includes[base_name]['files'].append(cpp_path)
        # Convert include paths to namespace-like format for comparison
        for inc in cpp_info['includes']:
            # kotlinx/coroutines/Job.hpp -> kotlinx.coroutines.Job
            ns_like = inc.replace('/', '.').replace('.hpp', '').replace('.h', '')
            cpp_includes[base_name]['includes'].add(ns_like)

    # Find files that exist in both Kotlin and C++ and compare their dependencies
    common_files = set(kotlin_imports.keys()) & set(cpp_includes.keys())

    print(f"\n### Dependency Comparison Summary:")
    print(f"  Files in Kotlin:      {len(kotlin_imports)}")
    print(f"  Files in C++:         {len(cpp_includes)}")
    print(f"  Common files:         {len(common_files)}")

    # Analyze dependency mismatches
    mismatches = []
    for file_base in sorted(common_files):
        kt_deps = kotlin_imports[file_base]['imports']
        cpp_deps = cpp_includes[file_base]['includes']

        # Normalize Kotlin imports to just package names (strip class names)
        kt_packages = set()
        for imp in kt_deps:
            # Try to extract just the package part
            # kotlinx.coroutines.channels.Channel -> kotlinx.coroutines.channels
            parts = imp.split('.')
            # Heuristic: if last part starts with capital, it's probably a class
            if len(parts) > 2 and parts[-1] and parts[-1][0].isupper():
                kt_packages.add('.'.join(parts[:-1]))
            else:
                kt_packages.add(imp)

        # Find dependencies that exist in Kotlin but not C++
        kt_only = kt_packages - cpp_deps
        cpp_only = cpp_deps - kt_packages

        # Filter out common stdlib/internal dependencies we don't care about
        kt_only = {d for d in kt_only if d.startswith('kotlinx.coroutines') and not d.endswith('.internal')}
        cpp_only = {d for d in cpp_only if d.startswith('kotlinx.coroutines') and not d.endswith('.internal')}

        if kt_only or cpp_only:
            mismatches.append({
                'file': file_base,
                'kt_files': kotlin_imports[file_base]['files'],
                'cpp_files': cpp_includes[file_base]['files'],
                'kt_only': kt_only,
                'cpp_only': cpp_only
            })

    if mismatches:
        print(f"\n### Files with dependency mismatches ({len(mismatches)}):")
        for i, mismatch in enumerate(mismatches[:20]):  # Limit output
            print(f"\n  {i+1}. {mismatch['file']}")
            print(f"     Kotlin: {mismatch['kt_files'][0][:60]}")
            print(f"     C++:    {mismatch['cpp_files'][0][:60]}")

            if mismatch['kt_only']:
                print(f"     ⚠️  Kotlin imports NOT in C++:")
                for dep in sorted(mismatch['kt_only'])[:5]:
                    print(f"         - {dep}")
                if len(mismatch['kt_only']) > 5:
                    print(f"         ... and {len(mismatch['kt_only']) - 5} more")

            if mismatch['cpp_only']:
                print(f"     ℹ️  C++ includes NOT in Kotlin:")
                for dep in sorted(mismatch['cpp_only'])[:5]:
                    print(f"         - {dep}")
                if len(mismatch['cpp_only']) > 5:
                    print(f"         ... and {len(mismatch['cpp_only']) - 5} more")

        if len(mismatches) > 20:
            print(f"\n  ... and {len(mismatches) - 20} more files with mismatches")
    else:
        print("\n✅ No significant dependency mismatches found!")

    return mismatches

def main():
    print("Analyzing Kotlin packages and C++ namespaces...")

    kotlin_pkgs, kotlin_files = extract_kotlin_packages()
    cpp_data = extract_cpp_namespaces()

    print(f"\nFound {len(kotlin_pkgs)} Kotlin packages")
    print(f"Found {len(kotlin_files)} Kotlin files")
    print(f"Found {len(cpp_data)} C++ files")

    # Check C++ namespace consistency
    mismatches, no_ns, correct = analyze_cpp_namespace_consistency(cpp_data)

    # Compare Kotlin packages to C++ folders
    compare_packages_to_folders(kotlin_pkgs, cpp_data)

    # Analyze import/include dependencies
    dep_mismatches = analyze_import_dependencies(kotlin_files, cpp_data)

    # Don't show file-by-file comparison unless requested (too verbose)
    # list_files_by_package(kotlin_pkgs, cpp_data)

    print("\n" + "="*80)
    print("FINAL SUMMARY")
    print("="*80)
    print(f"C++ files with CORRECT namespace:   {len(correct)}")
    print(f"C++ files with NO namespace:        {len(no_ns)}")
    print(f"C++ files with WRONG namespace:     {len(mismatches)}")
    print(f"Files with dependency mismatches:   {len(dep_mismatches)}")

if __name__ == "__main__":
    main()
