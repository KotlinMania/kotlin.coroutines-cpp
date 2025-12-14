#!/usr/bin/env python
"""
Analyze Kotlin packages vs C++ namespaces to find gaps and mismatches.
Also checks import/include dependencies between files.

Results are ORDERED BY DEPENDENCY COUNT - most-imported Kotlin files first.
"""

import os
import re
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).parent
KOTLIN_SRC = ROOT / "tmp" / "kotlinx.coroutines" / "kotlinx-coroutines-core"
CPP_SRC = ROOT / "src"
CPP_INCLUDE = ROOT / "include"

# =============================================================================
# DEPENDENCY GRAPH (shared with analyze_symbols.py and all_reports.py)
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
    file_imports = {}

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

            # Extract imports
            imports = re.findall(r'^import\s+([\w.]+)', content, re.MULTILINE)
            file_imports[f] = imports

        except Exception:
            pass

    # Build Graph
    graph = defaultdict(set)
    for f in kt_files:
        content = file_contents[f]
        pkg = file_pkg_map[f]
        imports = file_imports.get(f, [])

        # Explicit imports
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

    return sorted_kt, usage_count, file_classes, file_pkg_map, file_imports, file_contents


def extract_kotlin_packages(sorted_kt, usage_count, file_pkg_map, file_imports, file_contents):
    """Extract package declarations and imports from Kotlin files."""
    packages = defaultdict(list)
    kotlin_files = {}

    for kt_file in sorted_kt:
        rel_path = kt_file.relative_to(KOTLIN_SRC) if kt_file.is_relative_to(KOTLIN_SRC) else kt_file
        pkg = file_pkg_map.get(kt_file)
        content = file_contents.get(kt_file, '')

        if pkg:
            packages[pkg].append(str(rel_path))

        # Extract kotlinx.coroutines imports
        imports = []
        for imp in file_imports.get(kt_file, []):
            if imp.startswith('kotlinx.coroutines'):
                imports.append(imp)

        kotlin_files[str(rel_path)] = {
            'package': pkg,
            'imports': imports,
            'full_path': str(kt_file),
            'deps': usage_count.get(kt_file, 0)
        }

    return packages, kotlin_files


def folder_to_expected_namespace(folder_path):
    """
    Convert a folder path to the expected C++ namespace.
    Handles platform source sets (common, native, concurrent) by stripping them.
    """
    parts = folder_path.split('.')
    filtered = []
    i = 0
    while i < len(parts):
        part = parts[i]
        if part in ('common', 'native', 'concurrent'):
            if i + 1 < len(parts) and parts[i + 1] == 'test':
                i += 1
                continue
            i += 1
            continue
        if part == 'src':
            i += 1
            continue
        filtered.append(part)
        i += 1

    return '.'.join(filtered) if filtered else folder_path


def extract_cpp_namespaces():
    """Extract namespace declarations and includes from all .cpp/.hpp files."""
    namespaces = {}

    for search_root in [CPP_SRC, CPP_INCLUDE]:
        if not search_root.exists():
            continue

        for ext in ['*.cpp', '*.hpp']:
            for cpp_file in search_root.rglob(ext):
                if 'build' in str(cpp_file):
                    continue

                rel_path = cpp_file.relative_to(search_root)
                folder_path = str(rel_path.parent).replace('/', '.')

                with open(cpp_file, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()

                declared_ns = extract_deepest_namespace(content)
                expected_ns = folder_to_expected_namespace(folder_path)

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
    """Extract the deepest/most specific namespace from C++ content."""
    content_clean = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
    content_clean = re.sub(r'/\*.*?\*/', '', content_clean, flags=re.DOTALL)

    namespaces_found = []
    lines = content_clean.split('\n')
    current_ns_stack = []

    for line in lines:
        ns_match = re.match(r'\s*namespace\s+(\w+)\s*\{', line)
        if ns_match:
            ns_name = ns_match.group(1)
            if ns_name not in ('std', 'detail', 'anonymous'):
                current_ns_stack.append(ns_name)
                if current_ns_stack and current_ns_stack[0] == 'kotlinx':
                    namespaces_found.append('.'.join(current_ns_stack))

        ns17_match = re.match(r'\s*namespace\s+([\w:]+)\s*\{', line)
        if ns17_match and '::' in ns17_match.group(1):
            ns_path = ns17_match.group(1).replace('::', '.')
            if ns_path.startswith('kotlinx'):
                namespaces_found.append(ns_path)

        close_match = re.search(r'\}\s*//\s*namespace\s+(\w+)', line)
        if close_match and current_ns_stack:
            if current_ns_stack[-1] == close_match.group(1):
                current_ns_stack.pop()

    if namespaces_found:
        return max(namespaces_found, key=lambda x: x.count('.'))
    return None


def analyze_cpp_namespace_consistency(cpp_data, kotlin_files, usage_count):
    """Check if C++ files have namespaces matching their folder locations."""
    print("\n" + "="*80)
    print("C++ NAMESPACE vs FOLDER CONSISTENCY CHECK (ordered by dependency)")
    print("="*80)

    mismatches = []
    no_namespace = []
    correct = []

    # Build map from C++ file base name to Kotlin dependency count
    def get_dep_count(filepath):
        base = Path(filepath).stem.replace('.common', '').replace('.native', '')
        for kt_path, kt_info in kotlin_files.items():
            kt_base = Path(kt_path).stem.replace('.common', '').replace('.native', '')
            if kt_base == base:
                return kt_info.get('deps', 0)
        return 0

    for filepath, info in sorted(cpp_data.items(), key=lambda x: -get_dep_count(x[0])):
        folder = info['folder']
        expected = info['expected']
        declared = info['declared']

        if not folder.startswith('kotlinx.coroutines'):
            continue

        if any(x in folder for x in ['.tools.', '.integration.', 'tests.']):
            continue

        dep_count = get_dep_count(filepath)

        if not declared:
            no_namespace.append((filepath, expected, dep_count))
            continue

        if declared == expected:
            correct.append((filepath, expected, declared, dep_count))
        elif expected.startswith(declared + '.'):
            mismatches.append((filepath, expected, declared, 'PARENT_NS', dep_count))
        elif declared.startswith(expected + '.'):
            mismatches.append((filepath, expected, declared, 'CHILD_NS', dep_count))
        else:
            mismatches.append((filepath, expected, declared, 'DIFFERENT', dep_count))

    print(f"\n### Summary:")
    print(f"  Correct namespace:    {len(correct)}")
    print(f"  Missing namespace:    {len(no_namespace)}")
    print(f"  Mismatched namespace: {len(mismatches)}")

    if no_namespace:
        print(f"\n### Files with NO namespace declaration ({len(no_namespace)}):")
        for filepath, expected, deps in sorted(no_namespace, key=lambda x: -x[2]):
            dep_str = f" (deps: {deps})" if deps > 0 else ""
            print(f"  {filepath}{dep_str}")
            print(f"    Expected: {expected}")

    if mismatches:
        print(f"\n### Files with MISMATCHED namespace ({len(mismatches)}):")

        by_type = defaultdict(list)
        for item in mismatches:
            by_type[item[3]].append((item[0], item[1], item[2], item[4]))

        for mtype, items in sorted(by_type.items()):
            print(f"\n  [{mtype}] ({len(items)} files):")
            for filepath, expected, declared, deps in sorted(items, key=lambda x: -x[3]):
                dep_str = f" (deps: {deps})" if deps > 0 else ""
                print(f"    {filepath}{dep_str}")
                print(f"      Folder->Expected: {expected}")
                print(f"      Declared:         {declared}")

    return mismatches, no_namespace, correct


def analyze_import_dependencies(kotlin_files, cpp_data, usage_count):
    """Analyze and compare import/include dependencies, ordered by dependency count."""
    print("\n" + "="*80)
    print("IMPORT/INCLUDE DEPENDENCY ANALYSIS (ordered by dependency)")
    print("="*80)

    # Build maps
    kotlin_imports = {}
    for kt_path, kt_info in kotlin_files.items():
        base_name = os.path.basename(kt_path)
        base_name = os.path.splitext(base_name)[0]
        base_name = base_name.replace('.common', '').replace('.native', '').replace('.concurrent', '')

        if base_name not in kotlin_imports:
            kotlin_imports[base_name] = {
                'files': [], 'imports': set(), 'package': kt_info['package'],
                'deps': kt_info.get('deps', 0)
            }

        kotlin_imports[base_name]['files'].append(kt_path)
        kotlin_imports[base_name]['imports'].update(kt_info['imports'])
        # Take max deps across matching files
        kotlin_imports[base_name]['deps'] = max(
            kotlin_imports[base_name]['deps'],
            kt_info.get('deps', 0)
        )

    cpp_includes = {}
    for cpp_path, cpp_info in cpp_data.items():
        base_name = cpp_info['filename']
        base_name = os.path.splitext(base_name)[0]
        base_name = base_name.replace('.common', '').replace('.native', '').replace('.concurrent', '')

        if base_name not in cpp_includes:
            cpp_includes[base_name] = {'files': [], 'includes': set(), 'namespace': cpp_info['declared']}

        cpp_includes[base_name]['files'].append(cpp_path)
        for inc in cpp_info['includes']:
            ns_like = inc.replace('/', '.').replace('.hpp', '').replace('.h', '')
            cpp_includes[base_name]['includes'].add(ns_like)

    common_files = set(kotlin_imports.keys()) & set(cpp_includes.keys())

    print(f"\n### Dependency Comparison Summary:")
    print(f"  Files in Kotlin:      {len(kotlin_imports)}")
    print(f"  Files in C++:         {len(cpp_includes)}")
    print(f"  Common files:         {len(common_files)}")

    mismatches = []
    for file_base in common_files:
        kt_deps = kotlin_imports[file_base]['imports']
        cpp_deps = cpp_includes[file_base]['includes']
        deps = kotlin_imports[file_base].get('deps', 0)

        kt_packages = set()
        for imp in kt_deps:
            parts = imp.split('.')
            if len(parts) > 2 and parts[-1] and parts[-1][0].isupper():
                kt_packages.add('.'.join(parts[:-1]))
            else:
                kt_packages.add(imp)

        kt_only = kt_packages - cpp_deps
        cpp_only = cpp_deps - kt_packages

        kt_only = {d for d in kt_only if d.startswith('kotlinx.coroutines') and not d.endswith('.internal')}
        cpp_only = {d for d in cpp_only if d.startswith('kotlinx.coroutines') and not d.endswith('.internal')}

        if kt_only or cpp_only:
            mismatches.append({
                'file': file_base,
                'kt_files': kotlin_imports[file_base]['files'],
                'cpp_files': cpp_includes[file_base]['files'],
                'kt_only': kt_only,
                'cpp_only': cpp_only,
                'deps': deps
            })

    # Sort by dependency count
    mismatches.sort(key=lambda x: -x['deps'])

    if mismatches:
        print(f"\n### Files with dependency mismatches ({len(mismatches)}):")
        print("    (Ordered by dependency count - most depended-on first)\n")

        for i, mismatch in enumerate(mismatches[:30]):
            dep_str = f" [deps: {mismatch['deps']}]" if mismatch['deps'] > 0 else ""
            print(f"  {i+1}. {mismatch['file']}{dep_str}")
            print(f"     Kotlin: {mismatch['kt_files'][0][:60]}")
            print(f"     C++:    {mismatch['cpp_files'][0][:60]}")

            if mismatch['kt_only']:
                print(f"     Missing from C++:")
                for dep in sorted(mismatch['kt_only'])[:5]:
                    print(f"         - {dep}")
                if len(mismatch['kt_only']) > 5:
                    print(f"         ... and {len(mismatch['kt_only']) - 5} more")

            if mismatch['cpp_only']:
                print(f"     Extra in C++ (not in Kotlin):")
                for dep in sorted(mismatch['cpp_only'])[:5]:
                    print(f"         + {dep}")
                if len(mismatch['cpp_only']) > 5:
                    print(f"         ... and {len(mismatch['cpp_only']) - 5} more")
            print()

        if len(mismatches) > 30:
            print(f"  ... and {len(mismatches) - 30} more files with mismatches")
    else:
        print("\n All dependencies match!")

    return mismatches


def main():
    print("Analyzing Kotlin packages and C++ namespaces...")
    print("Building Kotlin dependency graph...")

    sorted_kt, usage_count, file_classes, file_pkg_map, file_imports, file_contents = build_kotlin_dependency_graph()
    print(f"Analyzed {len(sorted_kt)} Kotlin files\n")

    # Show top dependencies
    print("Top 10 most-depended-on Kotlin files:")
    for kt_file in sorted_kt[:10]:
        deps = usage_count.get(kt_file, 0)
        name = kt_file.name
        print(f"  {deps:3d} deps: {name}")
    print()

    kotlin_pkgs, kotlin_files = extract_kotlin_packages(
        sorted_kt, usage_count, file_pkg_map, file_imports, file_contents
    )
    cpp_data = extract_cpp_namespaces()

    print(f"Found {len(kotlin_pkgs)} Kotlin packages")
    print(f"Found {len(kotlin_files)} Kotlin files")
    print(f"Found {len(cpp_data)} C++ files")

    # Check C++ namespace consistency
    mismatches, no_ns, correct = analyze_cpp_namespace_consistency(cpp_data, kotlin_files, usage_count)

    # Analyze import/include dependencies
    dep_mismatches = analyze_import_dependencies(kotlin_files, cpp_data, usage_count)

    print("\n" + "="*80)
    print("FINAL SUMMARY")
    print("="*80)
    print(f"C++ files with CORRECT namespace:   {len(correct)}")
    print(f"C++ files with NO namespace:        {len(no_ns)}")
    print(f"C++ files with WRONG namespace:     {len(mismatches)}")
    print(f"Files with dependency mismatches:   {len(dep_mismatches)}")


if __name__ == "__main__":
    main()
