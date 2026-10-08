// Test fixture: assertions and trace output are confined to this executable.
#include "kotlinx/coroutines/KotlinGCBridge.hpp"
#include "kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/internal/TypeInfoNames.hpp"

#include <cassert>
#include <cstdio>
#include <optional>
#include <stdexcept>
#include <string>

extern "C" {
ObjHeader* NativeTypeNames_create(std::int32_t index, ObjHeader** result);
ObjHeader* NativeTypeNames_expected(ObjHeader* value, std::int32_t kind, ObjHeader** result);
void NativeTypeNames_collect();
const void* Kotlin_Any_getTypeInfo(const ObjHeader* value) __attribute__((nothrow));
std::int32_t Kotlin_String_getStringLength(const ObjHeader* value);
std::uint16_t Kotlin_String_get(const ObjHeader* value, std::int32_t index);
}

namespace {
std::optional<std::u16string> expected_text(const ObjHeader* text) {
  if (text == nullptr) return std::nullopt;
  std::u16string result;
  const auto length = Kotlin_String_getStringLength(text);
  for (std::int32_t index = 0; index < length; ++index) {
    result.push_back(Kotlin_String_get(text, index));
  }
  return result;
}

void trace(std::int32_t index, std::int32_t round, std::int32_t kind,
           const std::optional<std::u16string>& text) {
  std::printf("name %d %d %d ", index, round, kind);
  if (!text) std::printf("null");
  else for (const auto unit : *text) std::printf("%04x", static_cast<unsigned>(unit));
  std::printf("\n");
}
}  // namespace

extern "C" std::int32_t NativeTypeNames_cppContract() {
  using kotlin::native::internal::TypeInfoNames;
  try {
    TypeInfoNames invalid(nullptr);
    assert(false);
  } catch (const std::invalid_argument& error) {
    assert(std::string(error.what()) == "Failed requirement.");
  }
  std::int32_t observations = 0;
  for (std::int32_t index = 0; index < 12; ++index) {
    ObjHolder object;
    NativeTypeNames_create(index, object.slot());
    const auto* metadata = static_cast<const TypeInfo*>(Kotlin_Any_getTypeInfo(object.obj()));
    assert(metadata != nullptr);
    const TypeInfoNames names(metadata);
    for (std::int32_t round = 0; round < 2; ++round) {
      if (round == 1) NativeTypeNames_collect();
      assert(Kotlin_Any_getTypeInfo(object.obj()) == metadata);
      for (std::int32_t kind = 0; kind < 3; ++kind) {
        auto* const original_frame = getCurrentFrame();
        const auto actual = kind == 0 ? names.simple_name()
                           : kind == 1 ? names.qualified_name() : names.full_name();
        assert(getCurrentFrame() == original_frame);
        ObjHolder expected;
        const auto* expected_value = NativeTypeNames_expected(object.obj(), kind, expected.slot());
        assert(actual == expected_text(expected_value));
        // Specific source distinctions, using actual compiler-generated classes.
        if (index == 10 && kind == 0) assert(actual == u"Deeper");
        if (index == 11) assert(actual == u"UnpackagedName");
        if (index == 9 && kind == 0) assert(actual == u"Δ雪😀");
        trace(index, round, kind, actual);
        ++observations;
      }
    }
  }
  return observations;
}
