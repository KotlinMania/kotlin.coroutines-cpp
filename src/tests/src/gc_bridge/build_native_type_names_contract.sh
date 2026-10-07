#!/bin/bash
# Actual Native metadata fixture; tools are supplied explicitly.
set -euo pipefail
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 KONANC NATIVE_CLANGXX BUILD_DIR" >&2
    exit 2
fi
TYPE_NAMES_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TYPE_NAMES_PROJECT_ROOT="$(cd "$TYPE_NAMES_SCRIPT_DIR/../../../.." && pwd)"
TYPE_NAMES_KONANC="$1"
TYPE_NAMES_CLANGXX="$2"
mkdir -p "$3"
TYPE_NAMES_BUILD_DIR="$(cd "$3" && pwd)"
TYPE_NAMES_NATIVE_ROOT="$(cd "$(dirname "$TYPE_NAMES_KONANC")/.." && pwd)"
TYPE_NAMES_STDLIB="$TYPE_NAMES_NATIVE_ROOT/klib/common/stdlib"
TYPE_NAMES_SDK="$(xcrun --show-sdk-path)"
TYPE_NAMES_FLAGS=(-std=c++20 -Wall -Wextra -Werror -UNDEBUG -mmacosx-version-min=12.0
    -isysroot "$TYPE_NAMES_SDK" -isystem "$TYPE_NAMES_SDK/usr/include/c++/v1"
    -I "$TYPE_NAMES_PROJECT_ROOT/src" -emit-llvm -c)
"$TYPE_NAMES_CLANGXX" "${TYPE_NAMES_FLAGS[@]}" "$TYPE_NAMES_PROJECT_ROOT/src/kotlinx/coroutines/KotlinGCBridge.cpp" -o "$TYPE_NAMES_BUILD_DIR/type-names-bridge.bc"
"$TYPE_NAMES_CLANGXX" "${TYPE_NAMES_FLAGS[@]}" "$TYPE_NAMES_PROJECT_ROOT/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/internal/TypeInfoNames.cpp" -o "$TYPE_NAMES_BUILD_DIR/type-names.bc"
"$TYPE_NAMES_CLANGXX" "${TYPE_NAMES_FLAGS[@]}" "$TYPE_NAMES_PROJECT_ROOT/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/internal/TypeInfoNamesNative.cpp" -o "$TYPE_NAMES_BUILD_DIR/type-names-native.bc"
"$TYPE_NAMES_CLANGXX" "${TYPE_NAMES_FLAGS[@]}" "$TYPE_NAMES_SCRIPT_DIR/NativeTypeNamesContract.cpp" -o "$TYPE_NAMES_BUILD_DIR/type-names-contract.bc"
"$TYPE_NAMES_KONANC" -target macos_arm64 -entry docking.metadata.main -Xallow-kotlin-package \
    -opt-in=kotlin.native.internal.InternalForKotlinNative \
    -friend-modules "$TYPE_NAMES_STDLIB" -library "$TYPE_NAMES_STDLIB" \
    -native-library "$TYPE_NAMES_BUILD_DIR/type-names-bridge.bc" \
    -native-library "$TYPE_NAMES_BUILD_DIR/type-names.bc" \
    -native-library "$TYPE_NAMES_BUILD_DIR/type-names-native.bc" \
    -native-library "$TYPE_NAMES_BUILD_DIR/type-names-contract.bc" \
    "$TYPE_NAMES_PROJECT_ROOT/tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt" \
    "$TYPE_NAMES_SCRIPT_DIR/NativeTypeNamesContract.kt" \
    "$TYPE_NAMES_SCRIPT_DIR/NativeTypeNamesUnpackaged.kt" \
    -o "$TYPE_NAMES_BUILD_DIR/native_type_names_contract"
