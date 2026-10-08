// port-lint: source native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:12-48
#pragma once
#include "org/jetbrains/kotlin/name/FqName.hpp"
#include <string_view>
namespace org::jetbrains::kotlin::backend::konan {
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:13-13
inline constexpr std::u16string_view NATIVE_PTR_NAME = u"NativePtr";
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:16-16
inline constexpr std::u16string_view NON_NULL_NATIVE_PTR_NAME = u"NonNullNativePtr";
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:19-19
inline constexpr std::u16string_view IMMUTABLE_BLOB_OF = u"immutableBlobOf";
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:21-48
class KonanFqNames final {
public:
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:22-22
    static const name::FqName& function();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:23-23
    static const name::FqName& k_function();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:24-24
    static const name::FqName& package_name();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:25-25
    static const name::FqName& internal_package_name();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:26-26
    static const name::FqNameUnsafe& native_ptr();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:27-27
    static const name::FqNameUnsafe& non_null_native_ptr();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:28-28
    static const name::FqName& vector128();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:29-29
    static const name::FqName& throws();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:30-30
    static const name::FqName& cancellation_exception();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:31-31
    static const name::FqName& thread_local_name();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:32-32
    static const name::FqName& volatile_name();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:33-33
    static const name::FqName& can_be_precreated();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:34-34
    static const name::FqName& typed_intrinsic();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:35-35
    static const name::FqName& constant_constructor_intrinsic();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:36-36
    static const name::FqName& obj_c_method();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:37-37
    static const name::FqName& gc_unsafe_call();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:38-38
    static const name::FqName& eager_initialization();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:39-39
    static const name::FqName& no_reorder_fields();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:40-40
    static const name::FqName& obj_c_name();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:41-41
    static const name::FqName& obj_c_enum();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:42-42
    static const name::FqName& obj_c_enum_entry_name();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:43-43
    static const name::FqName& hides_from_obj_c();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:44-44
    static const name::FqName& refines_in_swift();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:45-45
    static const name::FqName& should_refine_in_swift();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:46-46
    static const name::FqName& no_inline();
    // Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/KonanFqNames.kt:47-47
    static const name::FqName& transparent_for_debugger();
};
}
