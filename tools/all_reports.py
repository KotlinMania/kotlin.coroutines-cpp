#!/usr/bin/env python3
"""
Unified reporting tool for kotlin.coroutines-cpp.

Functionality:
1. Analysis: Scans codebase for dependencies, TODOs, lint errors, and structure.
2. Structure: Reports results ORDERED BY DEPENDENCY IMPORTANCE (most depended-on files first).
3. Detail: Provides per-file breakdown of status, issues, and progress.

Usage:
    python tools/all_reports.py [options]

Options:
    --all           Run full report (default if no args)
    --compact       Show summary only, hide individual TODOs/Errors
"""

import os
import re
import sys
import argparse
from pathlib import Path
from typing import List, Dict, Set, Any, Tuple
from collections import defaultdict

# =============================================================================
# CONSTANTS & CONFIG
# =============================================================================

PROJECT_ROOT = Path(__file__).parent.parent
CPP_SRC = PROJECT_ROOT / "src"
KOTLIN_DIR = PROJECT_ROOT / "tmp" / "kotlinx.coroutines" / "kotlinx-coroutines-core"

CPP_EXTENSIONS = {'.hpp', '.cpp', '.h'}

IGNORED_UNUSED_KEYWORDS = {
    'if', 'while', 'for', 'switch', 'catch', 'when', 'return', 
    'sizeof', 'alignof', 'decltype', 'static_assert', 'constexpr', 'template', 
    'void', 'int', 'bool', 'float', 'double', 'char', 'short', 'long', 'unsigned',
    'class_simple_name', 'hex_address', 'resumePoints', 
    'coroutine_scope', 'with_timeout', 'run_test', 
    'runBlocking', 'check_exception', 'launch' 
}

# =============================================================================
# DATA GATHERING MODULES
# =============================================================================

class FileInfo:
    def __init__(self, path: Path):
        self.path = path
        self.rel_path = str(path.relative_to(PROJECT_ROOT)) if path.is_absolute() else str(path)
        self.todos: List[str] = []
        self.lint_errors: List[str] = []
        self.structure_errors: List[str] = []
        self.symbol_errors: List[str] = []
        self.is_stub = False
        self.lines_cpp = 0
        self.lines_kt = 0
        
    @property
    def has_issues(self):
        return bool(self.todos or self.lint_errors or self.structure_errors or self.symbol_errors)

class Analyzer:
    def __init__(self):
        self.cpp_data: Dict[Path, FileInfo] = {} # Map Path -> FileInfo
        self.kt_files_by_dep: List[Dict] = []    # List of {path, deps, cpp_maps}

    def get_or_create_info(self, path: Path) -> FileInfo:
        if path not in self.cpp_data:
            self.cpp_data[path] = FileInfo(path)
        return self.cpp_data[path]

    # --- 1. Dependencies ---
    def analyze_dependencies(self):
        # Scan Kotlin files
        kt_files = []
        class_to_file = {}
        file_pkg_map = {}
        pkg_to_files = defaultdict(list)
        file_contents = {}

        for f in KOTLIN_DIR.rglob("*.kt"):
            if 'test' in str(f).lower(): continue
            try:
                content = f.read_text(errors='ignore')
                pkg_m = re.search(r'^package\s+([\w.]+)', content, re.MULTILINE)
                if not pkg_m: continue
                pkg = pkg_m.group(1)
                
                kt_files.append(f)
                file_pkg_map[f] = pkg
                pkg_to_files[pkg].append(f)
                file_contents[f] = content
                
                # Extract classes
                for m in re.finditer(r'(?:class|interface|object)\s+(\w+)', content):
                    class_to_file[f"{pkg}.{m.group(1)}"] = f
                    
            except: pass

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
                    if dep != f: graph[f].add(dep)
                elif imp.endswith('.*'):
                    target_pkg = imp[:-2]
                    for neighbor in pkg_to_files.get(target_pkg, []):
                        if neighbor != f: graph[f].add(neighbor)

        # Scan Kotlin files
        kt_files = []
        class_to_file = {}
        file_pkg_map = {}
        pkg_to_files = defaultdict(list)
        file_contents = {}
        file_classes = defaultdict(list) # store classes defined in each file

        for f in KOTLIN_DIR.rglob("*.kt"):
            if 'test' in str(f).lower(): continue
            try:
                content = f.read_text(errors='ignore')
                pkg_m = re.search(r'^package\s+([\w.]+)', content, re.MULTILINE)
                if not pkg_m: continue
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
                    
            except: pass

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
                    if dep != f: graph[f].add(dep)
                elif imp.endswith('.*'):
                    target_pkg = imp[:-2]
                    for neighbor in pkg_to_files.get(target_pkg, []):
                        if neighbor != f:
                             # Check usage of classes from wildcard neighbor
                             for cls in file_classes[neighbor]:
                                 if re.search(rf'\\b{cls}\\b', content):
                                      graph[f].add(neighbor)
                                      break

            # Implicit same-package
            for neighbor in pkg_to_files[pkg]:
                if neighbor == f: continue
                # Check usage of classes from same-package neighbor
                for cls in file_classes[neighbor]:
                    if re.search(rf'\\b{cls}\\b', content):
                        graph[f].add(neighbor)
                        break

        # Count
        usage_count = defaultdict(int)
        for consumer, providers in graph.items():
            for provider in providers:
                usage_count[provider] += 1

        # Sort
        sorted_kt = sorted(kt_files, key=lambda f: (-usage_count[f], str(f)))
        
        # Map to C++
        for kt_f in sorted_kt:
            base = kt_f.stem
            cpp_matches = []
            
            # Direct name match
            for cpp in CPP_SRC.rglob(f"{base}.*"):
                 if cpp.suffix in CPP_EXTENSIONS: cpp_matches.append(cpp)
            
            # Snake case fallback
            if not cpp_matches:
                snake = re.sub(r'(?<!^)(?=[A-Z])', '_', base).lower()
                for cpp in CPP_SRC.rglob(f"{snake}.*"):
                     if cpp.suffix in CPP_EXTENSIONS: cpp_matches.append(cpp)
            
            self.kt_files_by_dep.append({
                'kt_path': kt_f,
                'deps': usage_count[kt_f],
                'cpp_paths': cpp_matches
            })

    # --- 2. TODOs ---
    def scan_todos(self):
        for root, _, files in os.walk(CPP_SRC):
            for fname in files:
                if Path(fname).suffix in CPP_EXTENSIONS:
                    path = Path(root) / fname
                    info = self.get_or_create_info(path)
                    try:
                        with open(path, 'r', encoding='utf-8', errors='replace') as f:
                            for i, line in enumerate(f):
                                if m := re.search(r'//\s*TODO(?:\([^)]*\))?:\s*(.+)', line):
                                    info.todos.append(f"L{i+1}: {m.group(1).strip()}")
                    except: pass

    # --- 3. Linter ---
    def run_linter(self):
        for ext in ['*.cpp', '*.hpp']:
            for path in CPP_SRC.rglob(ext):
                if 'vendor' in str(path) or 'plugin' in str(path): continue
                self._lint_file(path)

    def _lint_file(self, path: Path):
        info = self.get_or_create_info(path)
        try:
            content = path.read_text(errors='ignore')
        except: return

        # Unused Params
        # Regex to capture function signature and body start
        # Limited to simple cases to avoid parser complexity
        func_re = re.compile(r'(\w+)\s*\(([^)]*)\)\s*(?:const\s*)?(?:noexcept\s*)?\{', re.MULTILINE)
        
        for m in func_re.finditer(content):
            name = m.group(1)
            if name in IGNORED_UNUSED_KEYWORDS: continue
            
            args_str = m.group(2)
            start = m.end()
            
            # Extract body (simple bracket count)
            depth = 1
            idx = start
            while idx < len(content) and depth > 0:
                if content[idx] == '{': depth += 1
                elif content[idx] == '}': depth -= 1
                idx += 1
            
            if depth != 0: continue # Failed to parse body
            body = content[start:idx-1]
            
            # Check args
            if not args_str.strip(): continue
            for arg in args_str.split(','):
                tokens = re.split(r'[\s\*&]+', arg.strip().split('=')[0]) # drop default values
                if not tokens: continue
                param = tokens[-1]
                
                if not param or param.startswith('/*') or param == 'void' or param.startswith('[['): continue
                
                # Check usage
                if not re.search(rf'\b{re.escape(param)}\b', body):
                    if f"(void){param}" not in body and f"(void) {param}" not in body:
                        info.lint_errors.append(f"Unused param '{param}' in '{name}'")

    # --- 4. Structure & Symbols ---
    def check_structure_and_symbols(self):
        for path in self.cpp_data.keys():
            info = self.cpp_data[path]
            try:
                content = path.read_text(errors='ignore')
                info.lines_cpp = content.count('\n') + 1
                
                # Header Guards
                if path.suffix == '.hpp':
                    if "#pragma once" not in content and "#ifndef" not in content:
                        info.structure_errors.append("Missing header guard")
                
                # Stubs
                clean = re.sub(r'//.*', '', content)
                clean = re.sub(r'#include.*', '', clean)
                clean = re.sub(r'namespace.*', '', clean)
                if len(clean.strip()) < 50:
                    info.is_stub = True
                    info.symbol_errors.append("File appears to be a stub")
                
                # Check mapping data to get Kotlin lines
                # (This is inefficient, O(N^2) effectively, but N is small ~200)
                kt_len = 0
                for entry in self.kt_files_by_dep:
                    if path in entry['cpp_paths']:
                        try:
                            kt_content = entry['kt_path'].read_text(errors='ignore')
                            kt_len = kt_content.count('\n') + 1
                            info.lines_kt = kt_len
                        except: pass
                        break
                
                if kt_len > 50 and info.lines_cpp > 0:
                    ratio = info.lines_cpp / kt_len
                    if ratio < 0.10:
                        info.structure_errors.append(f"Low Code Ratio: {ratio:.0%}")
                        
            except: pass

# =============================================================================
# REPORTER
# =============================================================================

def print_report(analyzer: Analyzer, compact: bool):
    print("\n" + "="*80)
    print(f"{'#':<4} {'DEPS':<6} {'CPP FILE':<40} {'STATUS':<25}")
    print("="*80)
    
    # Track which C++ files we've shown to list orphans later
    shown_cpp = set()
    
    total_todos = 0
    total_lint = 0
    
    for i, entry in enumerate(analyzer.kt_files_by_dep, 1):
        deps = entry['deps']
        kt_name = entry['kt_path'].name
        cpp_paths = entry['cpp_paths']
        
        if not cpp_paths:
            # Kotlin only
            print(f"{i:<4} {deps:<6} (Kotlin: {kt_name:<30}) {'⚠️  NO CPP IMPLEMENTATION':<25}")
            continue
            
        for cpp_path in cpp_paths:
            shown_cpp.add(cpp_path)
            info = analyzer.get_or_create_info(cpp_path)
            
            # Status line
            status_parts = []
            if info.is_stub: status_parts.append("STUB")
            if info.structure_errors: status_parts.append("STRUCT_ERR")
            if info.lint_errors: status_parts.append(f"LINT({len(info.lint_errors)})")
            if info.todos: status_parts.append(f"TODO({len(info.todos)})")
            
            status_str = ", ".join(status_parts) if status_parts else "✅ OK"
            if info.lines_kt > 0:
                ratio = info.lines_cpp / info.lines_kt
                ratio_str = f"{ratio:.0%}"
            else:
                ratio_str = "??"

            total_todos += len(info.todos)
            total_lint += len(info.lint_errors)
            
            rel = info.rel_path
            if len(rel) > 40: rel = "..." + rel[-37:]
            
            print(f"{i:<4} {deps:<6} {rel:<40} {status_str:<25}")
            
            # Detailed breakdown
            if not compact and info.has_issues:
                prefix = "      "
                # Code Ratio
                if info.lines_kt > 0 and (info.lines_cpp / info.lines_kt < 0.10):
                     print(f"{prefix}🔴 Low Code Ratio: {info.lines_cpp} lines vs {info.lines_kt} (Kotlin)")

                # Structure & Symbols
                for err in info.structure_errors: print(f"{prefix}❌ {err}")
                for err in info.symbol_errors: print(f"{prefix}⚠️  {err}")

                # Lint (Limit 5)
                for j, err in enumerate(info.lint_errors):
                    if j >= 5: 
                        print(f"{prefix}... and {len(info.lint_errors)-5} more lint errors")
                        break
                    print(f"{prefix}🔸 {err}")

                # Todos (Limit 3)
                for j, todo in enumerate(info.todos):
                    if j >= 3: 
                        print(f"{prefix}... and {len(info.todos)-3} more TODOs")
                        break
                    print(f"{prefix}📝 {todo}")
                
            if not compact and info.has_issues:
                print(f"{'':<6}{'-'*74}")

    # Orphans
    orphans = [p for p in analyzer.cpp_data if p not in shown_cpp]
    if orphans:
        print("\n" + "="*80)
        print("UNMAPPED C++ FILES (No inferred Kotlin source)")
        print("="*80)
        for p in sorted(orphans):
            info = analyzer.cpp_data[p]
            print(f"?    ?      {info.rel_path}")
            if not compact and info.has_issues:
                 for err in info.lint_errors[:3]: print(f"      🔸 {err}")
                 for todo in info.todos[:3]: print(f"      📝 {todo}")

    print("\n" + "="*80)
    print(f"SUMMARY: {total_todos} TODOs, {total_lint} Lint Errors")
    print("="*80)
    
    if total_lint > 0: sys.exit(1)

# =============================================================================
# MAIN
# =============================================================================

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--all', action='store_true')
    parser.add_argument('--compact', action='store_true')
    # Compatibility flags (noop now, effectively enabled by default/all)
    parser.add_argument('--todos', action='store_true')
    parser.add_argument('--lint', action='store_true')
    parser.add_argument('--dependencies', action='store_true')
    parser.add_argument('--validate', action='store_true')
    parser.add_argument('--symbols', action='store_true')
    
    args = parser.parse_args()
    
    analyzer = Analyzer()
    print("Analyzing dependencies...")
    analyzer.analyze_dependencies()
    print("Scanning TODOs...")
    analyzer.scan_todos()
    print("Running Linter...")
    analyzer.run_linter()
    print("Checking Structure...")
    analyzer.check_structure_and_symbols()
    
    print_report(analyzer, args.compact)

if __name__ == "__main__":
    main()
