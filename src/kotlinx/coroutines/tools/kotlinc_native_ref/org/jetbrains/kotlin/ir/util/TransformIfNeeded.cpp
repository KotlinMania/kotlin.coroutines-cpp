/*
 * Copyright 2010-2016 JetBrains s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:126-137
#include "Transform.hpp"
#include "../declarations/IrTypeParameter.hpp"
#include "../../../../../kotlin/collections/ArrayList.hpp"
namespace org::jetbrains::kotlin::ir::util {
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:126-137
std::shared_ptr<::kotlin::collections::List<declarations::IrTypeParameter*>> transform_if_needed(
    const std::shared_ptr<::kotlin::collections::List<declarations::IrTypeParameter*>>& list,
    visitors::detail::IrTransformerDispatch& transformer) {
  return detail::transform_if_needed_nodes(list, transformer);
}
}  // namespace org::jetbrains::kotlin::ir::util
