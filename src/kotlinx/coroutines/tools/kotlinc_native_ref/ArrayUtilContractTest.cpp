// Execute the actual compiler-owned array algorithms. This is not a Native ABI test.
#include "kotlin/collections/ArrayUtil.hpp"
#include "kotlin/collections/ArraysNative.hpp"
#include "org/jetbrains/kotlin/name/Name.hpp"
#include <cassert>
#include <iostream>
#include <limits>
#include <sstream>

using kotlin::Array;
using namespace kotlin::collections;
using Value = std::optional<std::int32_t>;

namespace {
Array<Value> numbers() {
  return Array<Value>(4, [](std::int32_t index) { return Value(index + 10); });
}
std::string contents(const Array<Value>& array) {
  std::ostringstream output;
  for (std::int32_t i = 0; i < array.get_size(); ++i) {
    if (i) output << ',';
    try {
      const auto value = array.get(i);
      if (value) output << *value;
      else output << "null-or-uninitialized";
    } catch (const std::bad_optional_access&) {
      // Source explicitly leaves uninitialized reads implementation-dependent.
      output << "null-or-uninitialized";
    }
  }
  return output.str();
}
std::string null_contents(const Array<Value>& array) {
  std::ostringstream output;
  for (std::int32_t i = 0; i < array.get_size(); ++i) {
    if (i) output << ',';
    const auto value = array.get(i);
    if (value) output << *value;
    else output << "null";
  }
  return output.str();
}
template <typename Operation>
std::string observe(Operation operation, bool messages = false) {
  try { return operation(); }
  catch (const std::invalid_argument& error) {
    return std::string("IllegalArgumentException") + (messages ? ":" + std::string(error.what()) : "");
  } catch (const std::out_of_range& error) {
    return std::string("IndexOutOfBoundsException") + (messages ? ":" + std::string(error.what()) : "");
  }
}
}

int main() {
  std::cout << std::boolalpha;
  for (std::int32_t size = -1; size <= 5; ++size) {
    std::cout << "allocate:" << size << '=' << observe([&] {
      auto array = array_of_uninitialized_elements<Value>(size);
      return std::to_string(array.get_size()) + ":" + contents(array);
    }, true) << '\n';
  }
  for (bool self : {false, true}) {
    for (std::int32_t from = -1; from <= 5; ++from) {
      for (std::int32_t to = -1; to <= 5; ++to) {
        for (std::int32_t count = -1; count <= 5; ++count) {
          auto source = numbers();
          auto destination = self ? source : Array<Value>(4, [](std::int32_t) { return Value(90); });
          const std::string before_source = contents(source);
          const std::string before_destination = contents(destination);
          const auto result = observe([&] {
            array_copy(source, from, destination, to, count);
            return contents(source) + '|' + contents(destination);
          });
          if (result == "IndexOutOfBoundsException") {
            assert(contents(source) == before_source && contents(destination) == before_destination);
          }
          std::cout << "copy:" << self << ':' << from << ':' << to << ':' << count << '=' << result << '\n';
        }
      }
    }
  }
  for (std::int32_t from = -1; from <= 5; ++from) {
    for (std::int32_t to = -1; to <= 5; ++to) {
      auto array = numbers();
      std::cout << "fill:" << from << ':' << to << '=' << observe([&] {
        array_fill(array, from, to, Value(71));
        return contents(array);
      }, true) << '\n';
      array = numbers();
      std::cout << "reset-range:" << from << ':' << to << '=' << observe([&] {
        reset_range(array, from, to);
        return contents(array);
      }, true) << '\n';
      std::cout << "slice:" << from << ':' << to << '=' << observe([&] {
        return contents(copy_of_uninitialized_elements(numbers(), from, to));
      }) << '\n';
    }
  }
  for (std::int32_t index = -1; index <= 4; ++index) {
    auto array = numbers();
    std::cout << "reset-at:" << index << '=' << observe([&] {
      reset_at(array, index);
      return contents(array);
    }) << '\n';
  }
  for (std::int32_t size = -1; size <= 7; ++size) {
    std::cout << "resize:" << size << '=' << observe([&] {
      return contents(copy_of_uninitialized_elements(numbers(), size));
    }) << '\n';
  }
  auto source = numbers();
  reset_at(source, 1);
  auto copied = copy_of_uninitialized_elements(source, 6);
  assert(!detail::ArrayStorageAccess::slot(copied, 1).has_value());
  assert(!detail::ArrayStorageAccess::slot(copied, 4).has_value());
  source.set(0, Value(99));
  assert(copied.get(0) == Value(10));
  std::cout << "copy-independent=" << contents(copied) << '\n';
  auto destination = numbers();
  auto& returned = copy_into(numbers(), destination);
  assert(&returned == &destination);
  std::cout << "default-copy=" << contents(destination) << ':' << (&returned == &destination) << '\n';
  std::cout << "overflow-copy=" << observe([&] {
    copy_into(source, destination, 0, -1, std::numeric_limits<std::int32_t>::max());
    return contents(destination);
  }) << '\n';
  std::cout << "overflow-slice=" << observe([&] {
    return contents(copy_of_uninitialized_elements(source, -1, std::numeric_limits<std::int32_t>::max()));
  }, true) << '\n';
  using org::jetbrains::kotlin::name::Name;
  auto names = array_of_uninitialized_elements<Name>(1);
  names.set(0, Name::identifier(u"real-name"));
  auto name_copy = copy_of_uninitialized_elements(names, 3);
  assert(name_copy.get(0).as_string() == u"real-name");
  reset_at(names, 0);
  assert(name_copy.get(0).as_string() == u"real-name");
  // A real translated class lacking a default constructor, not a fake map/node.
  std::cout << "non-default-constructed-value=true\n";
  auto references = array_of_uninitialized_elements<std::shared_ptr<Name>>(1);
  references.set(0, std::make_shared<Name>(Name::identifier(u"owned")));
  const auto* original = references.get(0).get();
  std::weak_ptr<Name> observed = references.get(0);
  auto reference_copy = copy_of_uninitialized_elements(references, 1);
  assert(reference_copy.get(0).get() == original);
  reset_at(references, 0);
  assert(!observed.expired());
  std::cout << "copy-retains-reference=" << !observed.expired() << '\n';
  reset_range(reference_copy, 0, 1);
  assert(observed.expired());
  std::cout << "reset-releases-reference=" << observed.expired() << '\n';
  for (std::int32_t size = -1; size <= 5; ++size) {
    std::cout << "allocate-null:" << size << '=' << observe([&] {
      auto array = kotlin::array_of_nulls<std::int32_t>(size);
      return std::to_string(array.get_size()) + ':' + null_contents(array);
    }, true) << '\n';
  }
  for (std::int32_t from = -1; from <= 5; ++from) {
    for (std::int32_t to = -1; to <= 5; ++to) {
      std::cout << "slice-null:" << from << ':' << to << '=' << observe([&] {
        return null_contents(copy_of_nulls(numbers(), from, to));
      }, true) << '\n';
    }
  }
  for (std::int32_t size = -1; size <= 7; ++size) {
    std::cout << "resize-null:" << size << '=' << observe([&] {
      return null_contents(copy_of(numbers(), size));
    }, true) << '\n';
  }
  std::cout << "overflow-null-slice=" << observe([&] {
    return null_contents(copy_of_nulls(numbers(), -1, std::numeric_limits<std::int32_t>::max()));
  }, true) << '\n';
  Array<std::int32_t> nonnullable(4, [](std::int32_t i) { return i + 10; });
  auto nullable_copy = copy_of(nonnullable, 6);
  nonnullable.set(0, 99);
  assert(nullable_copy.get(0) == Value(10));
  std::cout << "nonnull-to-nullable=" << null_contents(nullable_copy) << '\n';
  auto retained = kotlin::array_of_nulls<std::shared_ptr<Name>>(1);
  retained.set(0, std::make_shared<Name>(Name::identifier(u"attribute-owner")));
  const auto* retained_identity = retained.get(0).get();
  std::weak_ptr<Name> retained_observer = retained.get(0);
  auto grown = copy_of(retained, 5);
  assert(grown.get(0).get() == retained_identity);
  for (std::int32_t i = 1; i < 5; ++i) assert(!grown.get(i));
  retained.set(0, nullptr);
  assert(!retained_observer.expired());
  grown.set(0, nullptr);
  assert(retained_observer.expired());
  std::cout << "null-growth-reference=identity,readable-null-tail,retain,release\n";
  static_assert(std::same_as<decltype(kotlin::array_of_nulls<Value>(1)), Array<Value>>);
  static_assert(std::same_as<decltype(kotlin::array_of_nulls<Name*>(1)), Array<Name*>>);
  static_assert(std::same_as<decltype(kotlin::array_of_nulls<std::any>(1)), Array<std::any>>);
  const auto erased_name = Name::identifier(u"real-erased-key");
  auto erased = kotlin::array_of_nulls<std::any>(2);
  erased.set(0, std::any(&erased_name));
  auto erased_copy = copy_of(erased, 6);
  assert(std::any_cast<const Name*>(erased_copy.get(0)) == &erased_name);
  for (std::int32_t i = 1; i < 6; ++i) assert(!erased_copy.get(i).has_value());
  auto borrowed = kotlin::array_of_nulls<Name*>(3);
  for (std::int32_t i = 0; i < 3; ++i) assert(borrowed.get(i) == nullptr);
}
