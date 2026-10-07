// Exercises the complete ClassKind property domain consumed by IrClass.kind.
// No IR class, descriptor, symbol or metadata instance is fabricated.
#include "org/jetbrains/kotlin/descriptors/ClassKind.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <string>

namespace descriptors = org::jetbrains::kotlin::descriptors;

int main() {
  using descriptors::ClassKind;
  constexpr std::array values{ClassKind::CLASS, ClassKind::INTERFACE,
                             ClassKind::ENUM_CLASS, ClassKind::ENUM_ENTRY,
                             ClassKind::ANNOTATION_CLASS, ClassKind::OBJECT};
  constexpr std::array<const char*, 6> names{
      "CLASS", "INTERFACE", "ENUM_CLASS", "ENUM_ENTRY", "ANNOTATION_CLASS", "OBJECT"};
  constexpr std::array<const char*, 6> representations{
      "class", "interface", "enum class", "<null>", "annotation class", "object"};
  constexpr std::array<std::array<bool, 7>, 6> expected{{
      {{false, true, false, false, false, false, false}},
      {{false, false, true, false, false, false, false}},
      {{false, false, false, true, false, false, false}},
      {{true, false, false, false, true, false, false}},
      {{false, false, false, false, false, true, false}},
      {{true, false, false, false, false, false, true}},
  }};
  for (std::size_t i = 0; i < values.size(); ++i) {
    const auto value = values[i];
    const auto representation = descriptors::code_representation(value);
    const std::string text = representation
        ? std::string(representation->begin(), representation->end()) : "<null>";
    assert(text == representations[i]);
    assert(representation.has_value() == (i != 3));
    const std::array properties{
        descriptors::is_singleton(value), descriptors::is_class(value),
        descriptors::is_interface(value), descriptors::is_enum_class(value),
        descriptors::is_enum_entry(value), descriptors::is_annotation_class(value),
        descriptors::is_object(value)};
    assert(properties == expected[i]);
    std::cout << names[i] << '|' << text;
    for (bool property : properties) std::cout << '|' << property;
    std::cout << '\n';
  }
}
