#!/bin/bash
# Build the actual Native integer-array fixture with its matching LLVM.
set -euo pipefail
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 KONANC NATIVE_CLANGXX BUILD_DIR" >&2
    exit 2
fi
INT_ARRAY_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INT_ARRAY_PROJECT_ROOT="$(cd "$INT_ARRAY_SCRIPT_DIR/../../../.." && pwd)"
INT_ARRAY_KONANC="$1"
INT_ARRAY_CLANGXX="$2"
mkdir -p "$3"
INT_ARRAY_BUILD_DIR="$(cd "$3" && pwd)"
INT_ARRAY_NATIVE_ROOT="$(cd "$(dirname "$INT_ARRAY_KONANC")/.." && pwd)"
INT_ARRAY_STDLIB="$INT_ARRAY_NATIVE_ROOT/klib/common/stdlib"
INT_ARRAY_SDK="$(xcrun --show-sdk-path)"
INT_ARRAY_FLAGS=(-std=c++20 -Wall -Wextra -Werror -mmacosx-version-min=12.0
    -isysroot "$INT_ARRAY_SDK" -isystem "$INT_ARRAY_SDK/usr/include/c++/v1"
    -I "$INT_ARRAY_PROJECT_ROOT/src" -emit-llvm -c)
"$INT_ARRAY_CLANGXX" "${INT_ARRAY_FLAGS[@]}" "$INT_ARRAY_PROJECT_ROOT/src/kotlinx/coroutines/KotlinGCBridge.cpp" -o "$INT_ARRAY_BUILD_DIR/int-array-bridge.bc"
"$INT_ARRAY_CLANGXX" "${INT_ARRAY_FLAGS[@]}" "$INT_ARRAY_SCRIPT_DIR/NativeIntArrayContract.cpp" -o "$INT_ARRAY_BUILD_DIR/int-array-contract.bc"
"$INT_ARRAY_KONANC" -target macos_arm64 -friend-modules "$INT_ARRAY_STDLIB" -library "$INT_ARRAY_STDLIB" \
    -native-library "$INT_ARRAY_BUILD_DIR/int-array-bridge.bc" -native-library "$INT_ARRAY_BUILD_DIR/int-array-contract.bc" \
    "$INT_ARRAY_SCRIPT_DIR/NativeIntArrayContract.kt" -o "$INT_ARRAY_BUILD_DIR/native_int_array_contract"
