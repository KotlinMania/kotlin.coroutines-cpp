// Exercises the actual source enum operations and concrete API types only.
// No concrete IR type, declaration, symbol or annotation instance is fabricated.
#include "org/jetbrains/kotlin/ir/types/IrType.hpp"
#include "org/jetbrains/kotlin/ir/expressions/impl/IrGetValueImpl.hpp"
#include "org/jetbrains/kotlin/ir/expressions/impl/IrSuspensionPointImpl.hpp"
#include "org/jetbrains/kotlin/ir/declarations/IrVariable.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>
namespace ir = org::jetbrains::kotlin::ir;
namespace types = org::jetbrains::kotlin::types;
namespace model = types::model;
static_assert(std::is_abstract_v<ir::types::IrType>);
static_assert(std::is_abstract_v<ir::types::IrErrorType>);
static_assert(std::is_abstract_v<ir::types::IrDynamicType>);
static_assert(std::is_abstract_v<ir::types::IrSimpleType>);
static_assert(std::is_abstract_v<ir::types::IrTypeArgument>);
static_assert(std::is_abstract_v<ir::types::IrTypeProjection>);
static_assert(std::is_abstract_v<ir::types::IrStarProjection>);
static_assert(std::is_convertible_v<ir::types::IrSimpleType*, model::KotlinTypeMarker*>);
static_assert(std::is_convertible_v<ir::types::IrDynamicType*, model::KotlinTypeMarker*>);
static_assert(std::is_base_of_v<model::TypeArgumentListMarker, ir::types::IrSimpleType>);
static_assert(std::is_base_of_v<ir::declarations::IrAnnotationContainer, ir::types::IrType>);
static_assert(std::is_base_of_v<org::jetbrains::kotlin::mpp::TypeRefMarker, ir::types::IrType>);
static_assert(std::is_same_v<decltype(std::declval<ir::types::IrType&>().type()), ir::types::IrType&>);
static_assert(std::is_same_v<decltype(std::declval<ir::types::IrSimpleType&>().classifier()), ir::symbols::IrClassifierSymbol&>);
static_assert(std::is_same_v<decltype(std::declval<ir::types::IrSimpleType&>().arguments()), kotlin::collections::List<ir::types::IrTypeArgument*>&>);
static_assert(std::is_same_v<decltype(std::declval<ir::types::IrType&>().original_kotlin_type()), types::KotlinType*>);
static_assert(std::is_same_v<decltype(std::declval<ir::types::IrType&>().equals(nullptr)), bool>);
static_assert(std::is_same_v<decltype(std::declval<ir::types::IrType&>().hash_code()), std::int32_t>);
static_assert(std::is_same_v<decltype(std::declval<ir::expressions::impl::IrGetValueImpl&>().type()), ir::types::IrType&>);
static_assert(std::is_same_v<decltype(std::declval<ir::expressions::impl::IrSuspensionPointImpl&>().type()), ir::types::IrType&>);
namespace {
std::string_view name(types::Variance value) {
  switch (value) {
    case types::Variance::INVARIANT: return "INVARIANT";
    case types::Variance::IN_VARIANCE: return "IN_VARIANCE";
    case types::Variance::OUT_VARIANCE: return "OUT_VARIANCE";
  }
  __builtin_unreachable();
}
std::string text(std::u16string_view value) { return std::string(value.begin(), value.end()); }
}
int main() {
  const std::array values{types::Variance::INVARIANT, types::Variance::IN_VARIANCE, types::Variance::OUT_VARIANCE};
  for (auto value : values) {
    std::cout << "value|" << name(value) << '|' << text(types::label(value)) << '|'
              << types::allows_in_position(value) << '|' << types::allows_out_position(value) << '|'
              << name(types::opposite(value)) << '|' << text(types::to_string(value)) << '\n';
    assert(types::opposite(types::opposite(value)) == value);
    for (auto other : values) {
      std::cout << "pair|" << name(value) << '|' << name(other) << '|'
                << types::allows_position(value, other) << '|' << name(types::superpose(value, other)) << '\n';
      assert(types::superpose(value, other) == types::superpose(other, value));
    }
  }
  for (bool question : {false, true}) {
    const auto nullability = ir::types::from_has_question_mark(question);
    std::cout << "nullable|" << question << '|'
              << (nullability == ir::types::SimpleTypeNullability::MARKED_NULLABLE ? "MARKED_NULLABLE" : "NOT_SPECIFIED") << '\n';
    assert(nullability != ir::types::SimpleTypeNullability::DEFINITELY_NOT_NULL);
  }
}
