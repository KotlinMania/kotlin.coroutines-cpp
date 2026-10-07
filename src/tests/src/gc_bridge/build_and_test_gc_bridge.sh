#!/bin/bash
# Native runtime ABI verification. Toolchains must be supplied explicitly.
set -euo pipefail
if [[ $# -lt 3 || $# -gt 4 ]]; then
    echo "Usage: $0 KONANC NATIVE_CLANGXX BUILD_DIR [--build-only]" >&2
    exit 2
fi
GC_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GC_PROJECT_ROOT="$(cd "$GC_SCRIPT_DIR/../../../.." && pwd)"
GC_KONANC="$1"
GC_CLANGXX="$2"
mkdir -p "$3"
GC_BUILD_DIR="$(cd "$3" && pwd)"
GC_NATIVE_ROOT="$(cd "$(dirname "$GC_KONANC")/.." && pwd)"
GC_STDLIB="$GC_NATIVE_ROOT/klib/common/stdlib"
GC_SDK="$(xcrun --show-sdk-path)"
GC_FLAGS=(-std=c++20 -Wall -Wextra -Werror -mmacosx-version-min=12.0
    -isysroot "$GC_SDK" -isystem "$GC_SDK/usr/include/c++/v1"
    -I "$GC_PROJECT_ROOT/src" -emit-llvm -c)
"$GC_CLANGXX" "${GC_FLAGS[@]}" "$GC_PROJECT_ROOT/src/kotlinx/coroutines/KotlinGCBridge.cpp" -o "$GC_BUILD_DIR/bridge.bc"
"$GC_CLANGXX" "${GC_FLAGS[@]}" "$GC_PROJECT_ROOT/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/Runtime.cpp" -o "$GC_BUILD_DIR/identity-hash.bc"
"$GC_CLANGXX" "${GC_FLAGS[@]}" "$GC_SCRIPT_DIR/NativeReferenceContract.cpp" -o "$GC_BUILD_DIR/contract.bc"
"$GC_KONANC" -target macos_arm64 -friend-modules "$GC_STDLIB" -library "$GC_STDLIB" \
    -native-library "$GC_BUILD_DIR/bridge.bc" -native-library "$GC_BUILD_DIR/identity-hash.bc" \
    -native-library "$GC_BUILD_DIR/contract.bc" "$GC_SCRIPT_DIR/NativeReferenceContract.kt" \
    -o "$GC_BUILD_DIR/native_reference_contract"
if [[ $# -eq 3 ]]; then
    "$GC_BUILD_DIR/native_reference_contract.kexe"
elif [[ "$4" != --build-only ]]; then
    echo "Unknown option: $4" >&2
    exit 2
fi
