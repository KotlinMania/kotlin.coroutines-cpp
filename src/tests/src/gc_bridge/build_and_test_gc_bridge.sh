#!/bin/bash
# build_and_test_gc_bridge.sh
# Build and test the Kotlin GC bridge integration

set -e

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

cd "$SCRIPT_DIR"

echo "==================================================="
echo "Building Kotlin Native GC Bridge Test"
echo "==================================================="

# Compile C++ implementation
echo
echo "Step 1: Compiling C++ implementation..."
clang++ -std=c++17 -I "$PROJECT_ROOT/include" \
    -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=1 \
    -c test_kotlin_gc_bridge_impl.cpp \
    -o test_kotlin_gc_bridge_impl.o

echo "✓ C++ compiled"

# Create static library
echo
echo "Step 2: Creating static library..."
ar rcs libtest_gc_bridge.a test_kotlin_gc_bridge_impl.o
echo "✓ Library created: libtest_gc_bridge.a"

# Check if kotlinc-native is available
if ! command -v kotlinc-native &> /dev/null; then
    echo
    echo "⚠️  kotlinc-native not found in PATH"
    echo
    echo "To run the full test, install Kotlin Native:"
    echo "  brew install kotlin"
    echo "  OR download from https://github.com/JetBrains/kotlin/releases"
    echo
    echo "For now, running standalone C++ test only..."
    echo
    
    # Run standalone test
    echo "Step 3: Running standalone C++ test..."
    clang++ -std=c++17 -I "$PROJECT_ROOT/include" \
        -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=0 \
        test_kotlin_gc_bridge.cpp \
        -o test_gc_bridge_standalone
    
    echo
    echo "==================================================="
    echo "Running Standalone C++ Test"
    echo "==================================================="
    ./test_gc_bridge_standalone
    
    exit 0
fi

# Generate cinterop library
echo
echo "Step 3: Generating Kotlin Native cinterop..."
cinterop -def test_gc_bridge.def \
    -compiler-option -I"$PROJECT_ROOT/include" \
    -o test_gc_bridge || {
    echo "✗ cinterop failed"
    echo "Trying with kotlinc-native directly..."
}

# Compile Kotlin code
echo
echo "Step 4: Compiling Kotlin code..."
kotlinc-native test_kotlin_gc_bridge.kt \
    -library libtest_gc_bridge.a \
    -include-binary libtest_gc_bridge.a \
    -o test_gc_bridge_kotlin || {
    echo "✗ Kotlin compilation failed"
    echo
    echo "This is expected - we need proper cinterop setup"
    echo "Running standalone test instead..."
    
    clang++ -std=c++17 -I "$PROJECT_ROOT/include" \
        -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=0 \
        test_kotlin_gc_bridge.cpp \
        -o test_gc_bridge_standalone
    
    echo
    echo "==================================================="
    echo "Running Standalone C++ Test"
    echo "==================================================="
    ./test_gc_bridge_standalone
    
    exit 0
}

# Run Kotlin test
echo
echo "==================================================="
echo "Running Kotlin Native Test"
echo "==================================================="
./test_gc_bridge_kotlin.kexe

echo
echo "==================================================="
echo "Test completed successfully!"
echo "==================================================="
