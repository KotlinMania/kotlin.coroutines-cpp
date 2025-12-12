#!/usr/bin/env python
"""
Extract detailed import/include mismatches for fixing.
"""

import os
import re
from collections import defaultdict
from pathlib import Path
import json

# Import the analysis functions
import analyze_packages as ap

def main():
    print("Extracting import mismatches...")

    kotlin_pkgs, kotlin_files = ap.extract_kotlin_packages()
    cpp_data = ap.extract_cpp_namespaces()

    # Build maps
    kotlin_imports = {}
    for kt_path, kt_info in kotlin_files.items():
        base_name = os.path.basename(kt_path)
        base_name = os.path.splitext(base_name)[0]
        base_name = base_name.replace('.common', '').replace('.native', '').replace('.concurrent', '')

        if base_name not in kotlin_imports:
            kotlin_imports[base_name] = {'files': [], 'imports': set(), 'package': kt_info['package']}

        kotlin_imports[base_name]['files'].append(kt_path)
        kotlin_imports[base_name]['imports'].update(kt_info['imports'])

    cpp_includes = {}
    for cpp_path, cpp_info in cpp_data.items():
        base_name = cpp_info['filename']
        base_name = os.path.splitext(base_name)[0]
        base_name = base_name.replace('.common', '').replace('.native', '').replace('.concurrent', '')

        if base_name not in cpp_includes:
            cpp_includes[base_name] = {'files': [], 'includes': set(), 'namespace': cpp_info['declared'], 'full_paths': []}

        cpp_includes[base_name]['files'].append(cpp_path)
        cpp_includes[base_name]['full_paths'].append(cpp_info['full_path'])
        for inc in cpp_info['includes']:
            ns_like = inc.replace('/', '.').replace('.hpp', '').replace('.h', '')
            cpp_includes[base_name]['includes'].add(ns_like)

    # Find mismatches
    common_files = set(kotlin_imports.keys()) & set(cpp_includes.keys())

    results = []
    for file_base in sorted(common_files):
        kt_deps = kotlin_imports[file_base]['imports']
        cpp_deps = cpp_includes[file_base]['includes']

        # Normalize Kotlin imports
        kt_packages = set()
        kt_classes = {}  # package -> [classes]
        for imp in kt_deps:
            parts = imp.split('.')
            if len(parts) > 2 and parts[-1] and parts[-1][0].isupper():
                pkg = '.'.join(parts[:-1])
                cls = parts[-1]
                kt_packages.add(pkg)
                if pkg not in kt_classes:
                    kt_classes[pkg] = []
                kt_classes[pkg].append(cls)
            else:
                kt_packages.add(imp)

        kt_only = kt_packages - cpp_deps
        kt_only = {d for d in kt_only if d.startswith('kotlinx.coroutines')}

        if kt_only:
            result = {
                'file': file_base,
                'cpp_files': cpp_includes[file_base]['full_paths'],
                'missing_imports': sorted(kt_only),
                'classes': {pkg: kt_classes.get(pkg, []) for pkg in kt_only}
            }
            results.append(result)

    print(f"\nFound {len(results)} C++ files missing imports")

    # Output top 50 for processing
    print("\n" + "="*80)
    print("TOP FILES NEEDING IMPORTS")
    print("="*80)

    for i, result in enumerate(results[:50]):
        print(f"\n{i+1}. {result['file']}")
        for cpp_file in result['cpp_files']:
            print(f"   File: {cpp_file}")
        print(f"   Missing imports:")
        for imp in result['missing_imports']:
            classes = result['classes'].get(imp, [])
            if classes:
                print(f"      - {imp} (classes: {', '.join(classes)})")
            else:
                print(f"      - {imp}")

    # Save full results to JSON
    output_file = Path(__file__).parent / "import_mismatches.json"
    with open(output_file, 'w') as f:
        json.dump(results, f, indent=2)

    print(f"\n\nFull results saved to: {output_file}")
    print(f"Total files needing imports: {len(results)}")

if __name__ == "__main__":
    main()
