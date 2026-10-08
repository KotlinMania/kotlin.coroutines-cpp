// Source contracts: native/base/src/main/kotlin/org/jetbrains/kotlin/backend/konan/InlineClasses.kt:50-70
// Names: KonanFqNames.kt:12-47; NativeRuntimeNames.kt:11-12,49
#include "InlineClasses.hpp"
#include "KonanFqNames.hpp"
#include "org/jetbrains/kotlin/name/NativeRuntimeNames.hpp"
#include <array>
#include <cassert>
using namespace org::jetbrains::kotlin::backend::konan;
using org::jetbrains::kotlin::name::NativeRuntimeNames;
namespace {
// Exercise compiler catalog access before main, from another translation unit.
const auto INITIAL_ID = class_id(KonanPrimitiveType::CHAR);
}

int main() {
    assert(INITIAL_ID.as_single_fq_name().as_string() == u"kotlin.Char");
    const std::array<KonanPrimitiveType, 10> types = {
        KonanPrimitiveType::BOOLEAN, KonanPrimitiveType::CHAR, KonanPrimitiveType::BYTE,
        KonanPrimitiveType::SHORT, KonanPrimitiveType::INT, KonanPrimitiveType::LONG,
        KonanPrimitiveType::FLOAT, KonanPrimitiveType::DOUBLE,
        KonanPrimitiveType::NON_NULL_NATIVE_PTR, KonanPrimitiveType::VECTOR128};
    const std::array<PrimitiveBinaryType, 10> binaries = {
        PrimitiveBinaryType::BOOLEAN, PrimitiveBinaryType::SHORT, PrimitiveBinaryType::BYTE,
        PrimitiveBinaryType::SHORT, PrimitiveBinaryType::INT, PrimitiveBinaryType::LONG,
        PrimitiveBinaryType::FLOAT, PrimitiveBinaryType::DOUBLE,
        PrimitiveBinaryType::POINTER, PrimitiveBinaryType::VECTOR128};
    const std::array<std::u16string, 10> names = {
        u"kotlin.Boolean", u"kotlin.Char", u"kotlin.Byte", u"kotlin.Short", u"kotlin.Int",
        u"kotlin.Long", u"kotlin.Float", u"kotlin.Double",
        u"kotlin.native.internal.NonNullNativePtr", u"kotlinx.cinterop.Vector128"};
    for (std::size_t i = 0; i < types.size(); ++i) {
        assert(fq_name(types[i]).as_string() == names[i]);
        assert(class_id(types[i]).as_single_fq_name().as_string() == names[i]);
        assert(!class_id(types[i]).is_nested_class());
        assert(!class_id(types[i]).is_local());
        assert(binary_type(types[i]).type() == binaries[i]);
        assert(primitive_binary_type_or_null(binary_type(types[i])) == binaries[i]);
        assert(&class_id(types[i]) == &class_id(types[i]));
        assert(&binary_type(types[i]) == &binary_type(types[i]));
    }
    // Char and Short share a storage kind, but retain distinct source IDs/boxes.
    assert(!class_id(KonanPrimitiveType::CHAR).equals(class_id(KonanPrimitiveType::SHORT)));
    assert(&binary_type(KonanPrimitiveType::CHAR) != &binary_type(KonanPrimitiveType::SHORT));
    assert(KonanFqNames::native_ptr().as_string() == u"kotlin.native.internal.NativePtr");
    assert(KonanFqNames::native_ptr().short_name().as_string() == NATIVE_PTR_NAME);
    assert(KonanFqNames::non_null_native_ptr().short_name().as_string() == NON_NULL_NATIVE_PTR_NAME);
    assert(KonanFqNames::non_null_native_ptr().parent().to_safe().equals(KonanFqNames::internal_package_name()));
    assert(KonanFqNames::gc_unsafe_call().equals(
        NativeRuntimeNames::Annotations::gc_unsafe_call_class_id().as_single_fq_name()));
    assert(KonanFqNames::gc_unsafe_call().as_string() == u"kotlin.native.internal.GCUnsafeCall");
    assert(KonanFqNames::obj_c_enum_entry_name().as_string() == u"kotlin.native.ObjCEnum.EntryName");
    assert(KonanFqNames::volatile_name().as_string() == u"kotlin.concurrent.Volatile");
    assert(IMMUTABLE_BLOB_OF == u"immutableBlobOf");
}
