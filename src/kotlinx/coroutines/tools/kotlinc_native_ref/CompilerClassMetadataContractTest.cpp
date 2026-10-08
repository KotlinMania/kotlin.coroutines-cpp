// Test-only assertions use actual translated classes and Clang-emitted data.
#include "kotlin/Any.hpp"
#include "kotlin/native/internal/KClassImpl.hpp"

#include <cassert>
#include <charconv>
#include <cstdio>
#include <string>

const kotlin::native::internal::detail::CompilerClassInfo* metadata_from_other_unit();

int main() {
  using kotlin::Any;
  using kotlin::native::internal::KClassImpl;
  using kotlin::native::internal::get_object_type_info;
  using kotlin::native::internal::TypeInfoNames;
  Any object;
  Any other;
  const auto* any_info = get_object_type_info(object);
  assert(any_info != nullptr && any_info == get_object_type_info(other));
  assert(any_info->super_type == nullptr && any_info->implemented_interfaces_count == 0);
  assert(object.equals(&object) && !object.equals(&other) && !object.equals(nullptr));
  const KClassImpl<Any> any_class(any_info);
  const KClassImpl<Any> same_class(any_info);
  assert(any_class.simple_name() == u"Any");
  assert(any_class.qualified_name() == u"kotlin.Any");
  assert(any_class.to_string() == u"class kotlin.Any");
  assert(any_class.equals(&same_class) && !any_class.equals(&object));
  assert(!any_class.equals(nullptr));
  assert(any_class.hash_code() == same_class.hash_code());
  assert(any_class.is_instance(&object) && any_class.is_instance(&any_class));
  assert(!any_class.is_instance(nullptr));

  const auto* impl_info = get_object_type_info(any_class);
  assert(impl_info != any_info && impl_info->super_type == any_info);
  assert(impl_info == metadata_from_other_unit());
  const KClassImpl<Any> implementation_class(impl_info);
  assert(implementation_class.simple_name() == u"KClassImpl");
  assert(implementation_class.qualified_name() == u"kotlin.native.internal.KClassImpl");
  assert(implementation_class.is_instance(&any_class));
  assert(!implementation_class.is_instance(&object));
  assert(!implementation_class.equals(&any_class));

  const KClassImpl<kotlin::reflect::KClass<Any>> generic_class(impl_info);
  assert(get_object_type_info(generic_class) == impl_info);
  assert(generic_class.equals(&implementation_class));
  assert(impl_info->implemented_interfaces_count == 5);
  for (std::int32_t index = 0; index < impl_info->implemented_interfaces_count; ++index) {
    const auto* interface_info = impl_info->implemented_interfaces[index];
    assert((interface_info->flags & kotlin::native::internal::detail::TF_INTERFACE) != 0);
    const KClassImpl<Any> interface_class(interface_info);
    assert(interface_class.is_instance(&any_class));
    assert(!interface_class.is_instance(&object));
    assert(interface_class.qualified_name().has_value());
  }

  char digits[16];
  const auto encoded = std::to_chars(digits, digits + sizeof(digits),
                                    static_cast<std::uint32_t>(object.hash_code()), 16);
  assert(encoded.ec == std::errc());
  const std::u16string expected_digits(digits, encoded.ptr);
  assert(object.to_string() == u"kotlin.Any@" + expected_digits);
  assert(TypeInfoNames(*impl_info).full_name() == u"kotlin.native.internal.KClassImpl");
  std::puts("actual Any/KClass identity, names, subtype, hash and text executed across two translation units");
}
