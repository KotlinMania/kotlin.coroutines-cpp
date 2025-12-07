#!/usr/bin/env python3

import os
import re
import subprocess
from pathlib import Path

def find_stub_files():
    """Find all .cpp files with stub markers"""
    patterns = [
        r"Template implementation.*header",
        r"companion header", 
        r"Template implementations.*header",
        r"NOTE:.*class definitions.*companion header"
    ]
    
    stub_files = []
    result = subprocess.run(['find', '.', '-name', '*.cpp'], 
                          capture_output=True, text=True)
    
    for cpp_file in result.stdout.strip().split('\n'):
        if not cpp_file:
            continue
            
        with open(cpp_file, 'r') as f:
            content = f.read()
            
        for pattern in patterns:
            if re.search(pattern, content):
                # Extract the exact marker and referenced header
                lines = content.split('\n')
                marker_line = None
                header_path = None
                
                for i, line in enumerate(lines):
                    if re.search(pattern, line):
                        marker_line = line.strip()
                        # Look for header path in the same or next line
                        if 'include/' in line:
                            header_match = re.search(r'include/[^`]+', line)
                            if header_match:
                                header_path = header_match.group()
                        elif i+1 < len(lines) and 'include/' in lines[i+1]:
                            header_match = re.search(r'include/[^`]+', lines[i+1])
                            if header_match:
                                header_path = header_match.group()
                        break
                
                stub_files.append((cpp_file, marker_line, header_path))
                break
    
    return stub_files

def assess_header_content(header_path):
    """Assess header file content"""
    if not header_path or not os.path.exists(header_path):
        return "NOT_FOUND", 0, []
    
    with open(header_path, 'r') as f:
        lines = f.readlines()
    
    # Count non-comment, non-include, non-using lines
    substantive_lines = []
    stub_indicators = []
    disabled_content = False
    
    for line in lines:
        stripped = line.strip()
        
        # Skip comments, includes, using directives, empty lines, pragmas
        if (stripped.startswith('//') or 
            stripped.startswith('/*') or 
            stripped.startswith('*') or 
            stripped.startswith('#include') or 
            stripped.startswith('using ') or
            stripped.startswith('#pragma') or
            stripped.startswith('#endif') or
            stripped.startswith('#else') or
            not stripped):
            continue
            
        # Check for disabled content
        if '#if 0' in stripped:
            disabled_content = True
            continue
        elif disabled_content and '#if' in stripped and '#if 0' not in stripped:
            disabled_content = False
            continue
        elif disabled_content:
            continue
            
        # Check for stub indicators
        if any(indicator in stripped.lower() for indicator in ['stub', 'tbd', 'todo', 'placeholder']):
            stub_indicators.append(stripped)
            
        # Check for actual function/class definitions with bodies
        if ('{' in stripped and '}' not in stripped) or '= 0' in stripped or '= default' in stripped:
            substantive_lines.append(stripped)
        elif stripped and not stripped.endswith(';') and not stripped.startswith('namespace') and not stripped.startswith('}'):
            substantive_lines.append(stripped)
    
    # Determine category
    if len(substantive_lines) == 0:
        category = "A"  # Empty header
    elif len(substantive_lines) <= 5 and len(stub_indicators) > 0:
        category = "B"  # Nearly empty header
    else:
        category = "C"  # Substantive header
        
    return category, len(substantive_lines), stub_indicators

def main():
    print("=== Kotlin Coroutines C++ Stub File Analysis ===\n")
    
    stub_files = find_stub_files()
    
    category_a = []
    category_b = []
    category_c = []
    not_found = []
    
    print(f"Found {len(stub_files)} stubbed .cpp files\n")
    
    for cpp_file, marker, header_ref in stub_files:
        # Determine header path
        if header_ref:
            header_path = header_ref
        else:
            # Use heuristic: replace /src/ with /include/ and .cpp with .hpp
            header_path = cpp_file.replace('/src/', '/include/').replace('.cpp', '.hpp')
        
        category, line_count, stub_indicators = assess_header_content(header_path)
        
        result = {
            'cpp_file': cpp_file,
            'marker': marker,
            'header_path': header_path,
            'category': category,
            'line_count': line_count,
            'stub_indicators': stub_indicators
        }
        
        if category == "A":
            category_a.append(result)
        elif category == "B":
            category_b.append(result)
        elif category == "C":
            category_c.append(result)
        else:
            not_found.append(result)
    
    # Print results
    print("=== CATEGORY A: Stubbed .cpp + Empty Header ===")
    for item in category_a:
        print(f"\n📁 {item['cpp_file']}")
        print(f"   📄 Header: {item['header_path']}")
        print(f"   🔍 Marker: {item['marker']}")
        print(f"   📊 Substantive lines: {item['line_count']}")
    
    print(f"\n=== CATEGORY B: Stubbed .cpp + Nearly Empty Header ===")
    for item in category_b:
        print(f"\n📁 {item['cpp_file']}")
        print(f"   📄 Header: {item['header_path']}")
        print(f"   🔍 Marker: {item['marker']}")
        print(f"   📊 Substantive lines: {item['line_count']}")
        print(f"   ⚠️  Stub indicators: {item['stub_indicators']}")
    
    print(f"\n=== CATEGORY C: Stubbed .cpp + Substantive Header (OK) ===")
    for item in category_c:
        print(f"\n📁 {item['cpp_file']}")
        print(f"   📄 Header: {item['header_path']}")
        print(f"   🔍 Marker: {item['marker']}")
        print(f"   📊 Substantive lines: {item['line_count']}")
    
    if not_found:
        print(f"\n=== NOT FOUND: Headers could not be located ===")
        for item in not_found:
            print(f"\n📁 {item['cpp_file']}")
            print(f"   📄 Header: {item['header_path']}")
            print(f"   🔍 Marker: {item['marker']}")
    
    # Summary
    print(f"\n=== SUMMARY ===")
    print(f"Category A (Empty headers): {len(category_a)} files")
    print(f"Category B (Nearly empty headers): {len(category_b)} files") 
    print(f"Category C (Substantive headers): {len(category_c)} files")
    print(f"Not found: {len(not_found)} files")
    print(f"Total stubbed files: {len(stub_files)}")
    
    # Files needing attention
    problem_files = len(category_a) + len(category_b)
    if problem_files > 0:
        print(f"\n⚠️  FILES NEEDING ATTENTION: {problem_files}")
        print("These represent stubbed .cpp files with empty or minimal headers.")

if __name__ == "__main__":
    main()