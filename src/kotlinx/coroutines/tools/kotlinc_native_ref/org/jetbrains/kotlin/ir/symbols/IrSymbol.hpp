/*
 * Copyright 2010-2017 JetBrains s.r.o.
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
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:36-138
#pragma once

#include <concepts>
#include "../../mpp/DeclarationSymbolMarkers.hpp"

namespace org::jetbrains::kotlin::descriptors {
class DeclarationDescriptor;
}
namespace org::jetbrains::kotlin::ir::declarations {
class IrSymbolOwner;
}
namespace org::jetbrains::kotlin::ir::util {
class IdSignature;
}

namespace org::jetbrains::kotlin::ir::symbols {

/**
 * A special object that can be used to refer to [IrDeclaration]s and some other entities from IR nodes.
 *
 * For example, [IrCall] uses [IrSimpleFunctionSymbol] to refer to the [IrSimpleFunction] that is being called.
 *
 * **Q:** Why not just use the [IrSimpleFunction] class itself?
 *
 * **A:** Because a symbol, unlike a declaration, can be bound or unbound (see [isBound]).
 *
 * We need this distinction to work with IR before the linkage phase. In pre-linkage IR the symbols referencing declarations from
 * other modules are not yet bound.
 *
 * During the linkage phase, we collect all the unbound symbols in the IR tree and try to resolve them to the declarations they should
 * refer to. For unbound symbols for public declarations from other modules, [signature] is used to resolve those declarations.
 *
 * @see IdSignature
 * @see SymbolTable
 */
// NOTE(port): Construction-phase opt-in and descriptor-obsolescence annotations
// remain compiler metadata work. The methods retain their source contracts.
// Owner/descriptor/signature references borrow compiler-owned objects; these
// forward declarations refer to actual Kotlin types still being translated.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:36-108
class IrSymbol : public virtual mpp::DeclarationSymbolMarker {
 public:
  /**
     * The declaration that this symbol refers to if it's bound.
     *
     * If the symbol is unbound, throws [IllegalStateException].
     *
     * **Q:** Why we didn't make this property nullable instead of throwing an exception?
     *
     * **A:** Because we most often need to access a symbol's owner in lowerings, which happen after linkage, at which point all symbols
     * should be already bound. Declaring this property nullable would make working with it more difficult most of the time.
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:56-67
  declarations::IrSymbolOwner& owner() const;

  /**
     * If [hasDescriptor] is `true`, returns the [DeclarationDescriptor] of the declaration that this symbol was created for.
     * Otherwise, returns a dummy [IrBasedDeclarationDescriptor] that serves as a descriptor-like view to [owner].
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:69-74
  ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor& descriptor() const;

  /**
     * Returns `true` if this symbol was created from a [DeclarationDescriptor] either emitted by the K1 (aka classic) frontend,
     * or from deserialized metadata.
     *
     * @see descriptor
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:76-83
  virtual bool has_descriptor() const = 0;

  /**
     * Whether this symbol has already been resolved to its [owner].
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:85-88
  virtual bool is_bound() const = 0;

  /**
     * If this symbol refers to a publicly accessible declaration (from the binary artifact point of view),
     * returns the binary signature of that declaration.
     *
     * Otherwise, returns `null`.
     *
     * @see IdSignature.isPubliclyVisible
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:90-98
  virtual util::IdSignature* signature() const = 0;

  // Used to store signatures in private symbols for JS incremental compilation.
  /**
     * If this symbol refers to a local declaration, the signature of that declaration, otherwise `null`.
     *
     * @see IdSignature.isPubliclyVisible
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:101-107
  virtual util::IdSignature* private_signature() const = 0;
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:107-107
  virtual void set_private_signature(util::IdSignature* signature) = 0;

 protected:
  // NOTE(port): Typed declarations and their symbols refer to one another.
  // C++ cannot validate recursive narrowed virtual returns while either type
  // is incomplete. Source-named typed getters retain this abstract dispatch
  // on the actual root owner; they do not create another owner or binding.
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:56-67
  virtual declarations::IrSymbolOwner& owner_dispatch() const = 0;

  // NOTE(port): Value and bindable symbol interfaces narrow the descriptor
  // through sibling inheritance paths. Keep one abstract property dispatch
  // and typed public access to the same actual descriptor.
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:69-74
  virtual ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor& descriptor_dispatch() const = 0;

  // NOTE(port): Kotlin symbols are reference objects. Prevent C++ value copies
  // from manufacturing a second symbol with copied binding state.
  IrSymbol() = default;
  IrSymbol(const IrSymbol&) = delete;
  IrSymbol& operator=(const IrSymbol&) = delete;
};

/**
 * Whether this symbol refers to a publicly accessible declaration (from the binary artifact point of view).
 *
 * The symbol doesn't have to be bound.
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:110-116
bool is_public_api(const IrSymbol& symbol);

/**
 * A stricter-typed [IrSymbol] that allows to set the owner using the [bind] method. The owner can be set only once.
 *
 * In fact, any [IrSymbol] is [IrBindableSymbol], but having a non-generic interface like [IrSymbol] is sometimes useful.
 *
 * Only leaf interfaces in the symbol hierarchy inherit from this interface.
 */
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:118-138
template <typename Descriptor, typename Owner>
  requires std::derived_from<Descriptor, ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor> &&
           std::derived_from<Owner, declarations::IrSymbolOwner>
class IrBindableSymbol : public virtual IrSymbol {
 public:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:126-127
  // NOTE(port): Narrow the actual virtual root result to the source Owner.
  // The source upper bound above requires the genuine owner hierarchy.
  Owner& owner() const {
    return dynamic_cast<Owner&>(owner_dispatch());
  }
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:129-130
  Descriptor& descriptor() const {
    return dynamic_cast<Descriptor&>(descriptor_dispatch());
  }
  /**
     * Sets this symbol's owner.
     *
     * Throws [IllegalStateException] if this symbol has already been bound.
     */
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:132-137
  virtual void bind(Owner& owner) = 0;
};

}  // namespace org::jetbrains::kotlin::ir::symbols
