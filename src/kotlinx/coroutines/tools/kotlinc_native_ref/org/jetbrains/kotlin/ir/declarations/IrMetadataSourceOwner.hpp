/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMetadataSourceOwner.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMetadataSourceOwner.kt:27-34
#pragma once
#include "../IrElement.hpp"
namespace org::jetbrains::kotlin::ir::declarations { class MetadataSource; }

namespace org::jetbrains::kotlin::ir::declarations {
/**
 * An [IrElement] capable of holding something which backends can use to write
 * as the metadata for the declaration.
 *
 * Technically, it can even be ± an array of bytes, but right now it's usually the frontend representation of the declaration,
 * so a descriptor in case of K1, and [org.jetbrains.kotlin.fir.FirElement] in case of K2,
 * and the backend invokes a metadata serializer on it to obtain metadata and write it, for example, to `@kotlin.Metadata`
 * on JVM.
 *
 * In Kotlin/Native, [metadata] is used to store some LLVM-related stuff in an IR declaration,
 * but this is only for performance purposes (before it was done using simple maps).
 *
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.metadataSourceOwner]
 */
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMetadataSourceOwner.kt:27-34
class IrMetadataSourceOwner : public virtual IrElement {
 public:
  /**
   * The arbitrary metadata associated with this IR node.
   *
   * @see IrMetadataSourceOwner
   */
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMetadataSourceOwner.kt:33-33
  virtual MetadataSource* metadata() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrMetadataSourceOwner.kt:33-33
  virtual void set_metadata(MetadataSource* value) = 0;
};
}  // namespace org::jetbrains::kotlin::ir::declarations
