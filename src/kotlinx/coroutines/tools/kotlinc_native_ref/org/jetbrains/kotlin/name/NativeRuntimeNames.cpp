// port-lint: source core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:10-66
#include "NativeRuntimeNames.hpp"
#include "StandardClassIds.hpp"
namespace org::jetbrains::kotlin::name {
namespace {
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:11-11
const FqName& kotlin_native_package() {
    static const FqName VALUE = FqName(u"kotlin.native");
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:12-12
const FqName& kotlin_native_internal_package() {
    static const FqName VALUE = kotlin_native_package().child(Name::identifier(u"internal"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:65-65
CallableId callable_id(const std::u16string& name, const FqName& package_name) {
    return CallableId(package_name, Name::identifier(name));
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:66-66
CallableId callable_id(const std::u16string& name, const ClassId& class_id) {
    return CallableId(class_id, Name::identifier(name));
}
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:14-14
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::atomic_int() {
    static const ClassId VALUE = ClassId(StandardClassIds::base_concurrent_package(), Name::identifier(u"AtomicInt"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:15-15
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::atomic_long() {
    static const ClassId VALUE = ClassId(StandardClassIds::base_concurrent_package(), Name::identifier(u"AtomicLong"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:16-16
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::atomic_reference() {
    static const ClassId VALUE = ClassId(StandardClassIds::base_concurrent_package(), Name::identifier(u"AtomicReference"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:23-23
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::atomic_array() {
    static const ClassId VALUE = ClassId(StandardClassIds::base_concurrent_package(), Name::identifier(u"AtomicArray"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:24-24
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::atomic_int_array() {
    static const ClassId VALUE = ClassId(StandardClassIds::base_concurrent_package(), Name::identifier(u"AtomicIntArray"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:25-25
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::atomic_long_array() {
    static const ClassId VALUE = ClassId(StandardClassIds::base_concurrent_package(), Name::identifier(u"AtomicLongArray"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:33-33
// NOTE(port): Retained property initialized on first access.
const CallableId& NativeRuntimeNames::Callables::atomic_array() {
    static const CallableId VALUE = callable_id(u"AtomicArray", StandardClassIds::base_concurrent_package());
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:35-35
// NOTE(port): Retained property initialized on first access.
const CallableId& NativeRuntimeNames::Callables::atomic_reference_compare_and_set() {
    static const CallableId VALUE = callable_id(u"compareAndSet", NativeRuntimeNames::atomic_reference());
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:36-36
// NOTE(port): Retained property initialized on first access.
const CallableId& NativeRuntimeNames::Callables::atomic_reference_compare_and_exchange() {
    static const CallableId VALUE = callable_id(u"compareAndExchange", NativeRuntimeNames::atomic_reference());
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:37-37
// NOTE(port): Retained property initialized on first access.
const CallableId& NativeRuntimeNames::Callables::atomic_array_compare_and_set() {
    static const CallableId VALUE = callable_id(u"compareAndSet", NativeRuntimeNames::atomic_array());
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:38-38
// NOTE(port): Retained property initialized on first access.
const CallableId& NativeRuntimeNames::Callables::atomic_array_compare_and_exchange() {
    static const CallableId VALUE = callable_id(u"compareAndExchange", NativeRuntimeNames::atomic_array());
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:42-42
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::symbol_name_class_id() {
    static const ClassId VALUE = ClassId(kotlin_native_package(), Name::identifier(u"SymbolName"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:43-43
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::c_name_class_id() {
    static const ClassId VALUE = ClassId(kotlin_native_package(), Name::identifier(u"CName"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:44-44
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::exported_bridge_class_id() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package(), Name::identifier(u"ExportedBridge"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:45-45
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::imported_bridge_class_id() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package(), Name::identifier(u"ImportedBridge"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:46-46
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::export_for_cpp_runtime_class_id() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package(), Name::identifier(u"ExportForCppRuntime"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:47-47
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::export_for_compiler_class_id() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package(), Name::identifier(u"ExportForCompiler"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:48-48
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::export_type_info_class_id() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package(), Name::identifier(u"ExportTypeInfo"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:49-49
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::gc_unsafe_call_class_id() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package(), Name::identifier(u"GCUnsafeCall"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:50-50
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::throws() {
    static const ClassId VALUE = ClassId(StandardClassIds::base_kotlin_package(), Name::identifier(u"Throws"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:51-51
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::throws_alias() {
    static const ClassId VALUE = ClassId(kotlin_native_package(), Name::identifier(u"Throws"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:52-52
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::shared_immutable() {
    static const ClassId VALUE = ClassId(kotlin_native_package().child(Name::identifier(u"concurrent")), Name::identifier(u"SharedImmutable"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:53-53
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::shared_immutable_alias() {
    static const ClassId VALUE = ClassId(kotlin_native_package(), Name::identifier(u"SharedImmutable"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:54-54
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::thread_local_name() {
    static const ClassId VALUE = ClassId(kotlin_native_package().child(Name::identifier(u"concurrent")), Name::identifier(u"ThreadLocal"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:55-55
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::thread_local_alias() {
    static const ClassId VALUE = ClassId(kotlin_native_package(), Name::identifier(u"ThreadLocal"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:56-56
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::points_to() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package().child(Name::identifier(u"escapeAnalysis")), Name::identifier(u"PointsTo"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:57-57
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::escapes() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package().child(Name::identifier(u"escapeAnalysis")), Name::identifier(u"Escapes"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:58-58
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::escapes_nothing() {
    static const ClassId VALUE = NativeRuntimeNames::Annotations::escapes().create_nested_class_id(Name::identifier(u"Nothing"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:59-59
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::has_finalizer() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package(), Name::identifier(u"HasFinalizer"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:60-60
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::bind_class_to_obj_c_name() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package().child(Name::identifier(u"objc")), Name::identifier(u"BindClassToObjCName"));
    return VALUE;
}
// Transliterated from: core/compiler.common.native/src/org/jetbrains/kotlin/name/NativeRuntimeNames.kt:61-61
// NOTE(port): Retained property initialized on first access.
const ClassId& NativeRuntimeNames::Annotations::bind_reverse_bridge_to_method() {
    static const ClassId VALUE = ClassId(kotlin_native_internal_package().child(Name::identifier(u"objc")), Name::identifier(u"BindReverseBridgeToMethod"));
    return VALUE;
}
}
