// A distinct actual generic specialization in another translation unit must
// share the Kotlin source class's immutable metadata identity.
#include "kotlin/native/internal/KClassImpl.hpp"

const kotlin::native::internal::detail::CompilerClassInfo* metadata_from_other_unit() {
  using kotlin::native::internal::KClassImpl;
  kotlin::Any object;
  const KClassImpl<kotlin::reflect::KClass<kotlin::Any>> klass(
      kotlin::native::internal::get_object_type_info(object));
  return kotlin::native::internal::get_object_type_info(klass);
}
