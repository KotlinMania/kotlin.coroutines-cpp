// port-lint: source core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:10-12,41-49
#include "NativeRuntimeNames.hpp"
namespace org::jetbrains::kotlin::name {
namespace {
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:11-11
const FqName& kotlin_native_package() {
    static const FqName VALUE(u"kotlin.native");
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:12-12
const FqName& kotlin_native_internal_package() {
    static const FqName VALUE = kotlin_native_package().child(Name::identifier(u"internal"));
    return VALUE;
}
}
// NOTE(port): Object properties use local-static getters to preserve retained
// values without relying on C++ translation-unit initialization order.
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:49-49
const ClassId& NativeRuntimeNames::Annotations::gc_unsafe_call_class_id() {
    static const ClassId VALUE(kotlin_native_internal_package(), Name::identifier(u"GCUnsafeCall"));
    return VALUE;
}
}
