// port-lint: source core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:10-66
#pragma once
#include "CallableId.hpp"
namespace org::jetbrains::kotlin::name {
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:10-63
// NOTE(port): Scalar names use retained getters; ThreadLocal escapes the
// C++ keyword through thread_local_name. Names do not install annotation effects.
class NativeRuntimeNames final {
public:
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:14-14
    static const ClassId& atomic_int();
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:15-15
    static const ClassId& atomic_long();
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:16-16
    static const ClassId& atomic_reference();
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:23-23
    static const ClassId& atomic_array();
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:24-24
    static const ClassId& atomic_int_array();
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:25-25
    static const ClassId& atomic_long_array();
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:32-32
    class Callables final {
    public:
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:33-33
        static const CallableId& atomic_array();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:35-35
        static const CallableId& atomic_reference_compare_and_set();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:36-36
        static const CallableId& atomic_reference_compare_and_exchange();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:37-37
        static const CallableId& atomic_array_compare_and_set();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:38-38
        static const CallableId& atomic_array_compare_and_exchange();
    };
    // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:41-41
    class Annotations final {
    public:
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:42-42
        static const ClassId& symbol_name_class_id();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:43-43
        static const ClassId& c_name_class_id();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:44-44
        static const ClassId& exported_bridge_class_id();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:45-45
        static const ClassId& imported_bridge_class_id();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:46-46
        static const ClassId& export_for_cpp_runtime_class_id();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:47-47
        static const ClassId& export_for_compiler_class_id();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:48-48
        static const ClassId& export_type_info_class_id();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:49-49
        static const ClassId& gc_unsafe_call_class_id();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:50-50
        static const ClassId& throws();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:51-51
        static const ClassId& throws_alias();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:52-52
        static const ClassId& shared_immutable();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:53-53
        static const ClassId& shared_immutable_alias();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:54-54
        static const ClassId& thread_local_name();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:55-55
        static const ClassId& thread_local_alias();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:56-56
        static const ClassId& points_to();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:57-57
        static const ClassId& escapes();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:58-58
        static const ClassId& escapes_nothing();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:59-59
        static const ClassId& has_finalizer();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:60-60
        static const ClassId& bind_class_to_obj_c_name();
        // Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:61-61
        static const ClassId& bind_reverse_bridge_to_method();
    };
};
}
