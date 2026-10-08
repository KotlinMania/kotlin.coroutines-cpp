#!/bin/bash
# Build the real Native array fixture with its matching LLVM compiler.
set -euo pipefail
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 KONANC NATIVE_CLANGXX BUILD_DIR" >&2
    exit 2
fi
ARRAY_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARRAY_PROJECT_ROOT="$(cd "$ARRAY_SCRIPT_DIR/../../../.." && pwd)"
ARRAY_KONANC="$1"
ARRAY_CLANGXX="$2"
mkdir -p "$3"
ARRAY_BUILD_DIR="$(cd "$3" && pwd)"
ARRAY_NATIVE_ROOT="$(cd "$(dirname "$ARRAY_KONANC")/.." && pwd)"
ARRAY_STDLIB="$ARRAY_NATIVE_ROOT/klib/common/stdlib"
ARRAY_SDK="$(xcrun --show-sdk-path)"
ARRAY_FLAGS=(-std=c++20 -Wall -Wextra -Werror -mmacosx-version-min=12.0
    -isysroot "$ARRAY_SDK" -isystem "$ARRAY_SDK/usr/include/c++/v1"
    -I "$ARRAY_PROJECT_ROOT/src" -emit-llvm -c)
"$ARRAY_CLANGXX" "${ARRAY_FLAGS[@]}" "$ARRAY_PROJECT_ROOT/src/kotlinx/coroutines/KotlinGCBridge.cpp" -o "$ARRAY_BUILD_DIR/array-bridge.bc"
"$ARRAY_CLANGXX" "${ARRAY_FLAGS[@]}" "$ARRAY_PROJECT_ROOT/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/collections/NativeArrayUtil.cpp" -o "$ARRAY_BUILD_DIR/native-array-util.bc"
"$ARRAY_CLANGXX" "${ARRAY_FLAGS[@]}" "$ARRAY_SCRIPT_DIR/NativeArrayUtilContract.cpp" -o "$ARRAY_BUILD_DIR/array-contract.bc"
"$ARRAY_KONANC" -target macos_arm64 -friend-modules "$ARRAY_STDLIB" -library "$ARRAY_STDLIB" \
    -native-library "$ARRAY_BUILD_DIR/array-bridge.bc" -native-library "$ARRAY_BUILD_DIR/native-array-util.bc" \
    -native-library "$ARRAY_BUILD_DIR/array-contract.bc" "$ARRAY_SCRIPT_DIR/NativeArrayUtilContract.kt" \
    -o "$ARRAY_BUILD_DIR/native_array_contract"
