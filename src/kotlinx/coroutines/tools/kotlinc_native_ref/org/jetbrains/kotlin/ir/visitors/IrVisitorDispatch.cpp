// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-86
#include "../IrElement.hpp"
namespace org::jetbrains::kotlin::ir::visitors::detail {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
void IrVisitorDispatch::accept(IrElement& element) { element.accept_dispatch(*this); }
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62
IrElement& IrTransformerDispatch::transform(IrElement& element) {
  element.transform_dispatch(*this);
  return take_element_result();
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:86-86
void IrTransformerDispatch::transform_children(IrElement& element) { element.transform_children_dispatch(*this); }
}  // namespace org::jetbrains::kotlin::ir::visitors::detail
