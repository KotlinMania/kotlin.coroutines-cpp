// port-lint: source core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:10-12,41-49
#pragma once
#include "ClassId.hpp"
namespace org::jetbrains::kotlin::name {
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:10-12,41-49
class NativeRuntimeNames final {
public:
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:41-49
    class Annotations final {
    public:
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:49-49
        static const ClassId& gc_unsafe_call_class_id();
    };
};
}
