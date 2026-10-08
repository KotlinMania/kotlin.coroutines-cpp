// C++ execution check for the Kotlin/Native iterator algorithm used by the
// compiler collection dependency. This does not execute Native object ABI code.
// Source: kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:88-92
#include "kotlin/Array.hpp"
#include "org/jetbrains/kotlin/descriptors/SourceElement.hpp"
#include "org/jetbrains/kotlin/name/Name.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

namespace {
template <typename T>
void exhaustion(kotlin::collections::Iterator<T>& iterator, const char* label,
                std::int32_t expected_index) {
  try {
    iterator.next();
    assert(false);
  } catch (const kotlinx::coroutines::NoSuchElementException& error) {
    assert(std::string(error.what()) == std::to_string(expected_index));
    std::cout << label << "=NoSuchElementException:" << error.what() << '\n';
  }
}
}

int main() {
  std::cout << std::boolalpha;
  std::ostringstream init_order;
  kotlin::Array<std::int32_t> array(3, [&](std::int32_t index) {
    if (index != 0) init_order << ',';
    init_order << index;
    return index + 10;
  });
  assert(init_order.str() == "0,1,2");
  std::cout << "init=" << init_order.str() << '\n';
  auto iterator = array.iterator();
  kotlin::collections::Iterator<std::any>& widened = *iterator;
  const bool same = dynamic_cast<void*>(&widened) == dynamic_cast<void*>(iterator.get());
  assert(same);
  std::cout << "same_iterator=" << same << '\n';
  auto first = iterator->next();
  assert(first == 10);
  std::cout << "typed_first=" << first << '\n';
  array.set(1, 77);
  auto second = std::any_cast<std::int32_t>(widened.next());
  assert(second == 77);
  std::cout << "widened_second=" << second << '\n';
  auto third = iterator->next();
  assert(third == 12);
  std::cout << "typed_third=" << third << '\n';
  assert(!iterator->has_next());
  std::cout << "has_next=" << iterator->has_next() << '\n';
  exhaustion(*iterator, "exhaustion_1", 3);
  exhaustion(widened, "exhaustion_2", 3);
  assert(!iterator->has_next());
  std::cout << "after_exhaustion_has_next=" << iterator->has_next() << '\n';
  auto fresh = array.iterator();
  std::cout << "fresh_first=" << fresh->next() << '\n';
  auto shared = array;
  shared.set(1, 88);
  auto shared_second = fresh->next();
  assert(shared_second == 88);
  std::cout << "shared_array_second=" << shared_second << '\n';
  auto retained = [] {
    kotlin::Array<std::int32_t> owner(1, [](std::int32_t) { return 31; });
    return owner.iterator();
  }();
  auto retained_value = retained->next();
  assert(retained_value == 31);
  std::cout << "retained_after_owner_scope=" << retained_value << '\n';
  kotlin::Array<std::int32_t> empty(0, [](std::int32_t) { assert(false); return 0; });
  auto empty_iterator = empty.iterator();
  std::cout << "empty_has_next=" << empty_iterator->has_next() << '\n';
  exhaustion(*empty_iterator, "empty_exhaustion_1", 0);
  exhaustion(*empty_iterator, "empty_exhaustion_2", 0);
  kotlin::Array<std::int32_t*> null_array(1, [](std::int32_t) { return nullptr; });
  auto null_iterator = null_array.iterator();
  auto* null_value = null_iterator->next();
  assert(null_value == nullptr);
  std::cout << "null_next=" << (null_value == nullptr) << '\n';
  using org::jetbrains::kotlin::name::Name;
  kotlin::Array<Name> names(2, [](std::int32_t index) {
    return index == 0 ? Name::identifier(u"alpha") : Name::special(u"<special>");
  });
  auto name_iterator = names.iterator();
  auto a = name_iterator->next().as_string();
  auto b = name_iterator->next().as_string();
  assert(a == u"alpha" && b == u"<special>");
  std::cout << "name_first=" << std::string(a.begin(), a.end()) << '\n';
  std::cout << "name_second=" << std::string(b.begin(), b.end()) << '\n';
  using org::jetbrains::kotlin::descriptors::SourceElement;
  kotlin::Array<const SourceElement*> sources(1, [](std::int32_t) { return &SourceElement::NO_SOURCE; });
  auto source_iterator = sources.iterator();
  const bool same_source = source_iterator->next() == &SourceElement::NO_SOURCE;
  assert(same_source);
  std::cout << "source_identity=" << same_source << '\n';
  try {
    kotlin::Array<std::int32_t> negative(-1, [](std::int32_t) { assert(false); return 0; });
    assert(false);
  } catch (const std::invalid_argument&) {
    std::cout << "negative_size=RuntimeException\n";
  }
  try { array.get(-1); assert(false); }
  catch (const std::out_of_range&) { std::cout << "negative_index=IndexOutOfBoundsException\n"; }
  try { array.set(3, 99); assert(false); }
  catch (const std::out_of_range&) { std::cout << "end_index=IndexOutOfBoundsException\n"; }
}
