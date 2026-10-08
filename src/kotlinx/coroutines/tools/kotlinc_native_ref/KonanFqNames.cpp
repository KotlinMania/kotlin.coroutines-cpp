// port-lint: source native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:12-48
#include "KonanFqNames.hpp"
#include "org/jetbrains/kotlin/name/NativeRuntimeNames.hpp"
namespace org::jetbrains::kotlin::backend::konan {
// NOTE(port): Object properties use local-static getters. volatile_name and
// thread_local_name map
// the Kotlin properties whose names are reserved C++ keywords.
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:22-22
const name::FqName& KonanFqNames::function() {
    static const name::FqName VALUE = name::FqName(u"kotlin.Function");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:23-23
const name::FqName& KonanFqNames::k_function() {
    static const name::FqName VALUE = name::FqName(u"kotlin.reflect.KFunction");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:24-24
const name::FqName& KonanFqNames::package_name() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:25-25
const name::FqName& KonanFqNames::internal_package_name() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.internal");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:26-26
const name::FqNameUnsafe& KonanFqNames::native_ptr() {
    static const name::FqNameUnsafe VALUE = internal_package_name().child(name::Name::identifier(std::u16string(NATIVE_PTR_NAME))).to_unsafe();
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:27-27
const name::FqNameUnsafe& KonanFqNames::non_null_native_ptr() {
    static const name::FqNameUnsafe VALUE = internal_package_name().child(name::Name::identifier(std::u16string(NON_NULL_NATIVE_PTR_NAME))).to_unsafe();
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:28-28
const name::FqName& KonanFqNames::vector128() {
    static const name::FqName VALUE = name::FqName(u"kotlinx.cinterop.Vector128");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:29-29
const name::FqName& KonanFqNames::throws() {
    static const name::FqName VALUE = name::FqName(u"kotlin.Throws");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:30-30
const name::FqName& KonanFqNames::cancellation_exception() {
    static const name::FqName VALUE = name::FqName(u"kotlin.coroutines.cancellation.CancellationException");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:31-31
const name::FqName& KonanFqNames::thread_local_name() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.concurrent.ThreadLocal");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:32-32
const name::FqName& KonanFqNames::volatile_name() {
    static const name::FqName VALUE = name::FqName(u"kotlin.concurrent.Volatile");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:33-33
const name::FqName& KonanFqNames::can_be_precreated() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.internal.CanBePrecreated");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:34-34
const name::FqName& KonanFqNames::typed_intrinsic() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.internal.TypedIntrinsic");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:35-35
const name::FqName& KonanFqNames::constant_constructor_intrinsic() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.internal.ConstantConstructorIntrinsic");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:36-36
const name::FqName& KonanFqNames::obj_c_method() {
    static const name::FqName VALUE = name::FqName(u"kotlinx.cinterop.ObjCMethod");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:37-37
const name::FqName& KonanFqNames::gc_unsafe_call() {
    static const name::FqName VALUE = name::NativeRuntimeNames::Annotations::gc_unsafe_call_class_id().as_single_fq_name();
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:38-38
const name::FqName& KonanFqNames::eager_initialization() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.EagerInitialization");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:39-39
const name::FqName& KonanFqNames::no_reorder_fields() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.internal.NoReorderFields");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:40-40
const name::FqName& KonanFqNames::obj_c_name() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.ObjCName");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:41-41
const name::FqName& KonanFqNames::obj_c_enum() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.ObjCEnum");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:42-42
const name::FqName& KonanFqNames::obj_c_enum_entry_name() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.ObjCEnum.EntryName");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:43-43
const name::FqName& KonanFqNames::hides_from_obj_c() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.HidesFromObjC");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:44-44
const name::FqName& KonanFqNames::refines_in_swift() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.RefinesInSwift");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:45-45
const name::FqName& KonanFqNames::should_refine_in_swift() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.ShouldRefineInSwift");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:46-46
const name::FqName& KonanFqNames::no_inline() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.NoInline");
    return VALUE;
}
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:47-47
const name::FqName& KonanFqNames::transparent_for_debugger() {
    static const name::FqName VALUE = name::FqName(u"kotlin.native.internal.TransparentForDebugger");
    return VALUE;
}
}
