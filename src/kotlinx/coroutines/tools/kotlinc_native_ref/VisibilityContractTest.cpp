// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:10-34
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibilities.kt:8-89
// Execution comparison for the actual source singleton objects, not a replacement hierarchy.
#include "org/jetbrains/kotlin/descriptors/Visibilities.hpp"
#include "org/jetbrains/kotlin/descriptors/DescriptorVisibility.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace {

// These source diagnostics contain only ASCII; compare their emitted code units.
std::string ascii(const std::u16string& value) {
  return std::string(value.begin(), value.end());
}

}  // namespace

int main() {
  using namespace org::jetbrains::kotlin::descriptors;
  static_assert(std::is_abstract_v<Visibility>);
  static_assert(std::is_abstract_v<DescriptorVisibility>);
  static_assert(!std::is_copy_constructible_v<Visibilities::Public>);
  const std::array<const Visibility*, 9> values = {
      &Visibilities::Private::INSTANCE, &Visibilities::PrivateToThis::INSTANCE,
      &Visibilities::Protected::INSTANCE, &Visibilities::Internal::INSTANCE,
      &Visibilities::Public::INSTANCE, &Visibilities::Local::INSTANCE,
      &Visibilities::Inherited::INSTANCE, &Visibilities::InvisibleFake::INSTANCE,
      &Visibilities::Unknown::INSTANCE};
  std::cout << std::boolalpha;
  for (const auto* value : values) {
    std::string imports;
    try {
      imports = value->must_check_in_imports() ? "true" : "false";
    } catch (const std::logic_error& error) {
      imports = error.what();
    }
    assert(&value->normalize() == value);
    assert(value->custom_effective_visibility() == nullptr);
    assert(value->to_string() == value->get_internal_display_name());
    std::cout << "visibility name=" << ascii(value->get_name())
              << " public=" << value->is_public_api() << " imports=" << imports
              << " internal=" << ascii(value->get_internal_display_name())
              << " external=" << ascii(value->get_external_display_name())
              << " normalized_same=" << (&value->normalize() == value)
              << " custom_null=" << (value->custom_effective_visibility() == nullptr)
              << " private=" << Visibilities::is_private(*value) << '\n';
  }
  for (const auto* first : values) {
    for (const auto* second : values) {
      const auto result = Visibilities::compare(*first, *second);
      std::cout << "compare first=" << ascii(first->get_name())
                << " second=" << ascii(second->get_name()) << " result=";
      if (result.has_value()) std::cout << *result;
      else std::cout << "null";
      std::cout << '\n';
    }
  }
  assert(!Visibilities::compare(Visibilities::Protected::INSTANCE,
                              Visibilities::Internal::INSTANCE).has_value());
  assert(!Visibilities::compare(Visibilities::Private::INSTANCE,
                              Visibilities::PrivateToThis::INSTANCE).has_value());
  assert(Visibilities::compare(Visibilities::Private::INSTANCE,
                             Visibilities::Public::INSTANCE) == -2);
  assert(&Visibilities::DEFAULT_VISIBILITY == &Visibilities::Public::INSTANCE);
  std::cout << "default_same="
            << (&Visibilities::DEFAULT_VISIBILITY == &Visibilities::Public::INSTANCE) << '\n';
}
