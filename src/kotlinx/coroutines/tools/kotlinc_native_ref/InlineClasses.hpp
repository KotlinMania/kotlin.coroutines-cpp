// port-lint: source native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-70
#pragma once
#include "BinaryType.hpp"
#include "org/jetbrains/kotlin/name/ClassId.hpp"
namespace org::jetbrains::kotlin::backend::konan {
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-60
enum class KonanPrimitiveType {
    BOOLEAN, CHAR, BYTE, SHORT, INT, LONG, FLOAT, DOUBLE, NON_NULL_NATIVE_PTR, VECTOR128
};
// NOTE(port): Property getters follow the existing Variance enum convention.
// Their references borrow the retained enum constructor properties.
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-50
const name::ClassId& class_id(KonanPrimitiveType type);
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-50
const BinaryType::Primitive& binary_type(KonanPrimitiveType type);
// Transliterated from: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:70-70
name::FqNameUnsafe fq_name(KonanPrimitiveType type);
}
