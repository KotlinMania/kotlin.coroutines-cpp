#!/usr/bin/env bash
#
# extract_llvm_deps.sh - Walk LLVM/Clang headers to find transitive dependencies
#
# Usage: ./extract_llvm_deps.sh <header_file>
#        ./extract_llvm_deps.sh --all   # Process all seed files
#
# Output: Prints list of all #include'd files transitively

set -e

LLVM_ROOT="${LLVM_ROOT:-/Volumes/stuff/Projects/kotlin.coroutines-cpp/vendor/llvm-project}"
OUTPUT_DIR="${OUTPUT_DIR:-/tmp/llvm_deps}"
VISITED_FILE="$OUTPUT_DIR/.visited"

mkdir -p "$OUTPUT_DIR"

# Seed files - the headers we need
SEED_FILES=(
    # LLVM IR
    "llvm/include/llvm/IR/IRBuilder.h"
    "llvm/include/llvm/IR/Instructions.h"
    "llvm/include/llvm/IR/Constants.h"
    "llvm/include/llvm/IR/BasicBlock.h"
    "llvm/include/llvm/IR/DerivedTypes.h"

    # Clang Analysis
    "clang/include/clang/Analysis/CFG.h"

    # Clang AST
    "clang/include/clang/AST/RecursiveASTVisitor.h"
    "clang/include/clang/AST/Decl.h"
    "clang/include/clang/AST/Type.h"

    # Clang Basic
    "clang/include/clang/Basic/TokenKinds.def"

    # Clang Sema
    "clang/include/clang/Sema/DeclSpec.h"
)

# Check if file was visited
is_visited() {
    grep -qxF "$1" "$VISITED_FILE" 2>/dev/null
}

# Mark file as visited
mark_visited() {
    echo "$1" >> "$VISITED_FILE"
}

# Resolve include path to actual file
resolve_include() {
    local include_path="$1"
    local from_file="$2"
    local from_dir=$(dirname "$from_file")

    # Try relative to current file first
    if [[ -f "$from_dir/$include_path" ]]; then
        realpath "$from_dir/$include_path"
        return 0
    fi

    # Try LLVM include paths
    for base in "$LLVM_ROOT/llvm/include" "$LLVM_ROOT/clang/include" "$LLVM_ROOT/llvm" "$LLVM_ROOT/clang"; do
        if [[ -f "$base/$include_path" ]]; then
            realpath "$base/$include_path"
            return 0
        fi
    done

    # Try build directory for generated headers
    if [[ -f "$LLVM_ROOT/build/include/$include_path" ]]; then
        realpath "$LLVM_ROOT/build/include/$include_path"
        return 0
    fi

    # Not found - might be system header
    return 1
}

# Extract #include directives from a file
extract_includes() {
    local file="$1"

    # Match #include "..." and #include <...>
    # Filter to only llvm/ and clang/ includes
    grep -E '^\s*#\s*include\s+[<"]' "$file" 2>/dev/null | \
        sed -E 's/.*#\s*include\s+[<"]([^">]+)[">].*/\1/' | \
        grep -E '^(llvm|clang)/' || true
}

# Recursively walk dependencies
walk_deps() {
    local file="$1"
    local depth="${2:-0}"

    # Normalize path
    if [[ -f "$file" ]]; then
        file=$(realpath "$file")
    fi

    # Skip if already visited
    if is_visited "$file"; then
        return 0
    fi
    mark_visited "$file"

    # Check file exists
    if [[ ! -f "$file" ]]; then
        return 0
    fi

    # Print this file
    echo "$file"

    # Get includes
    local includes=$(extract_includes "$file")

    # Recurse (limit depth to avoid infinite loops)
    if [[ $depth -lt 20 ]]; then
        for inc in $includes; do
            local resolved=$(resolve_include "$inc" "$file" 2>/dev/null || true)
            if [[ -n "$resolved" && -f "$resolved" ]]; then
                walk_deps "$resolved" $((depth + 1))
            fi
        done
    fi
}

# Process a single file
process_file() {
    local seed="$1"
    local full_path="$LLVM_ROOT/$seed"

    if [[ ! -f "$full_path" ]]; then
        echo "WARNING: Seed file not found: $full_path" >&2
        return 1
    fi

    echo "=== Processing: $seed ===" >&2
    walk_deps "$full_path"
}

# Main
if [[ "$1" == "--all" ]]; then
    # Clear visited file
    > "$VISITED_FILE"

    # Process all seed files
    ALL_DEPS="$OUTPUT_DIR/all_deps.txt"
    > "$ALL_DEPS"

    for seed in "${SEED_FILES[@]}"; do
        process_file "$seed" >> "$ALL_DEPS" 2>/dev/null || true
    done

    # Sort and dedupe
    sort -u "$ALL_DEPS" > "$OUTPUT_DIR/unique_deps.txt"

    echo "" >&2
    echo "=== Summary ===" >&2
    TOTAL=$(wc -l < "$OUTPUT_DIR/unique_deps.txt" | tr -d ' ')
    echo "Total unique files: $TOTAL" >&2
    echo "Output: $OUTPUT_DIR/unique_deps.txt" >&2

    # Categorize
    echo "" >&2
    echo "By category:" >&2
    echo "  LLVM IR:        $(grep -c '/llvm/IR/' "$OUTPUT_DIR/unique_deps.txt" 2>/dev/null || echo 0)" >&2
    echo "  LLVM ADT:       $(grep -c '/llvm/ADT/' "$OUTPUT_DIR/unique_deps.txt" 2>/dev/null || echo 0)" >&2
    echo "  LLVM Support:   $(grep -c '/llvm/Support/' "$OUTPUT_DIR/unique_deps.txt" 2>/dev/null || echo 0)" >&2
    echo "  Clang AST:      $(grep -c '/clang/AST/' "$OUTPUT_DIR/unique_deps.txt" 2>/dev/null || echo 0)" >&2
    echo "  Clang Basic:    $(grep -c '/clang/Basic/' "$OUTPUT_DIR/unique_deps.txt" 2>/dev/null || echo 0)" >&2
    echo "  Clang Sema:     $(grep -c '/clang/Sema/' "$OUTPUT_DIR/unique_deps.txt" 2>/dev/null || echo 0)" >&2
    echo "  Clang Analysis: $(grep -c '/clang/Analysis/' "$OUTPUT_DIR/unique_deps.txt" 2>/dev/null || echo 0)" >&2
    echo "  Clang Lex:      $(grep -c '/clang/Lex/' "$OUTPUT_DIR/unique_deps.txt" 2>/dev/null || echo 0)" >&2

    # Show the list
    echo "" >&2
    echo "=== Files ===" >&2
    cat "$OUTPUT_DIR/unique_deps.txt"

elif [[ -n "$1" ]]; then
    # Process single file
    > "$VISITED_FILE"
    process_file "$1"
else
    echo "Usage: $0 <header_file>"
    echo "       $0 --all"
    echo ""
    echo "Seed files:"
    for seed in "${SEED_FILES[@]}"; do
        echo "  $seed"
    done
fi
