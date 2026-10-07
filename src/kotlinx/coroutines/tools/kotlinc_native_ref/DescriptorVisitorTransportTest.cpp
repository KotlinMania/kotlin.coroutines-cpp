#include "org/jetbrains/kotlin/descriptors/DescriptorVisitorDispatch.hpp"
#include "org/jetbrains/kotlin/descriptors/SourceElement.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <type_traits>

using org::jetbrains::kotlin::descriptors::detail::DescriptorVisitorResult;

namespace {
struct NonDefaultResult {
  explicit NonDefaultResult(int value) : value(value) {}
  NonDefaultResult() = delete;
  int value;
};
}  // namespace

// Compile every typed virtual forwarding method for the source visitor's full
// fifteen-kind matrix. These instantiations use its real abstract contracts,
// without defining substitute descriptor classes.
template class org::jetbrains::kotlin::descriptors::detail::TypedDescriptorVisitorDispatch<int, int>;
template class org::jetbrains::kotlin::descriptors::detail::TypedDescriptorVisitorDispatch<int&, int*>;
template class org::jetbrains::kotlin::descriptors::detail::TypedDescriptorVisitorDispatch<std::unique_ptr<int>, std::nullptr_t>;
template class org::jetbrains::kotlin::descriptors::detail::TypedDescriptorVisitorDispatch<void, std::nullptr_t>;

// Exercise the language boundary's result transport without manufacturing a
// descriptor/declaration hierarchy. Actual visitor routing needs real descriptors.
int main() {
  static_assert(!std::is_default_constructible_v<NonDefaultResult>);
  DescriptorVisitorResult<NonDefaultResult> value;
  value.set(NonDefaultResult(42));
  assert(value.take().value == 42);

  auto owned = std::make_unique<int>(73);
  auto* identity = owned.get();
  DescriptorVisitorResult<std::unique_ptr<int>> move_only;
  move_only.set(std::move(owned));
  auto returned = move_only.take();
  assert(!owned);
  assert(returned.get() == identity);
  assert(*returned == 73);

  int context_value = 19;
  DescriptorVisitorResult<int&> reference;
  reference.set(context_value);
  assert(&reference.take() == &context_value);
  reference.take() = 21;
  assert(context_value == 21);

  DescriptorVisitorResult<const int&> const_reference;
  const_reference.set(context_value);
  assert(&const_reference.take() == &context_value);

  DescriptorVisitorResult<int*> nullable;
  nullable.set(nullptr);
  assert(nullable.take() == nullptr);

  DescriptorVisitorResult<void> unit;
  unit.take();
  // Use the real source-defined singleton objects, not test descriptor classes.
  using org::jetbrains::kotlin::descriptors::SourceElement;
  using org::jetbrains::kotlin::descriptors::SourceFile;
  const SourceElement& source = SourceElement::NO_SOURCE;
  const SourceFile& file = source.get_containing_file();
  assert(&file == &SourceFile::NO_SOURCE_FILE);
  assert(!file.get_name().has_value());
  assert(source.to_string() == "NO_SOURCE");
  assert(source.equals(std::cref(source)));
  assert(!source.equals(std::any{}));
  assert(!source.equals(std::cref(file)));
  assert(file.equals(std::cref(file)));
  assert(source.hash_code() == source.hash_code());
  assert(&SourceElement::NO_SOURCE == &source);
  assert(&source.get_containing_file() == &file);
  std::cout << "source=" << source.to_string()
            << " name=null same_file=true same_element=true\n";
  std::cout << "nondefault=42 move_identity=retained reference=21 nullable=null unit=returned\n";
}
