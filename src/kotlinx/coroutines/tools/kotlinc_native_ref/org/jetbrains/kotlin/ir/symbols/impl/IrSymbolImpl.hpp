/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:19-85
#pragma once

// NOTE(port): These are actual source dependencies, not substitute types.
// Their translation must precede instantiation of this source-generic class.
#include "../../../descriptors/DeclarationDescriptor.hpp"
#include "../../../descriptors/ValueParameterDescriptor.hpp"
#include "../../declarations/IrDeclaration.hpp"
#include "../../descriptors/IrBasedDescriptors.hpp"
#include "../../util/IdSignature.hpp"
#include "../../util/RenderIrElement.hpp"
#include "../IrSymbol.hpp"

#include <stdexcept>
#include <cstdint>
#include <sstream>
#include <string>
#include <typeinfo>

namespace org::jetbrains::kotlin::ir::symbols::impl {

// NOTE(port): The compiler owns descriptors, declarations and signatures.
// References here borrow those exact stable objects. C++ typeinfo supplies the
// diagnostic class spelling; Java reflection spelling remains a metadata gap.
// Public source generics require header definitions for their instantiations.
// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:19-80
template <typename Descriptor, typename Owner>
  requires std::derived_from<Descriptor, ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor> &&
           std::derived_from<Owner, declarations::IrSymbolOwner>
// NOTE(port): C++ must name the typed bind interface on this implementation
// base to implement it across the leaf symbol's multiple interface paths.
class IrSymbolBase : public virtual IrBindableSymbol<Descriptor, Owner> {
 public:
  // NOTE(port): A pure virtual destructor preserves the source abstract class.
  virtual ~IrSymbolBase() = 0;
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:20-23
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:44-54
  explicit IrSymbolBase(Descriptor* descriptor) : descriptor_(descriptor) {
    // NOTE(port): Preserve the source's assertion-enabled checks. C++ debug
    // builds enable them; assertion failures use the C++ exception boundary.
#ifndef NDEBUG
    if (descriptor != nullptr && !is_original_descriptor(*descriptor)) {
      throw std::logic_error("Substituted descriptor " + descriptor->to_string() +
                             " for " + descriptor->get_original().to_string());
    }
    if (!is_public_api(*this) && descriptor != nullptr) {
      auto* containing_declaration = descriptor->get_containing_declaration();
      if (containing_declaration != nullptr && !is_original_descriptor(*containing_declaration)) {
        throw std::logic_error("Substituted containing declaration: " + containing_declaration->to_string() +
                               "\nfor descriptor: " + descriptor->to_string());
      }
    }
#endif
  }

 protected:
  // NOTE(port): Typed source getters read this sole descriptor-selection body.
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:25-28
  ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor& descriptor_dispatch() const override final {
    if (descriptor_ != nullptr) return *descriptor_;
    return dynamic_cast<Descriptor&>(::org::jetbrains::kotlin::ir::descriptors::to_ir_based_descriptor(
        dynamic_cast<declarations::IrDeclaration&>(this->owner())));
  }

 public:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:30-32
  bool has_descriptor() const override { return descriptor_ != nullptr; }

 protected:
  // NOTE(port): The public typed getter invokes this sole source owner lookup.
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:35-37
  declarations::IrSymbolOwner& owner_dispatch() const override final {
    // Keep owner_ and the error separate, as upstream requests for breakpoints.
    if (owner_ != nullptr) return *owner_;
    throw std::logic_error(std::string(typeid(*this).name()) +
                           " is unbound. Signature: " + signature_string());
  }

 public:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:39-40
  util::IdSignature* signature() const override { return nullptr; }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:42-42
  util::IdSignature* private_signature() const override final { return private_signature_; }
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:42-42
  void set_private_signature(util::IdSignature* signature) override final { private_signature_ = signature; }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:61-62
  bool is_bound() const override final { return owner_ != nullptr; }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:64-70
  void bind(Owner& owner) override final {
    if (owner_ == nullptr) {
      owner_ = &owner;
    } else {
      throw std::logic_error(std::string(typeid(*this).name()) +
                             " is already bound. Signature: " + signature_string() +
                             ". Owner: " + util::render(*owner_));
    }
  }

  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:72-79
  virtual std::string to_string() const {
    if (is_bound()) return util::render(this->owner());
    if (is_public_api(*this)) {
      return std::string("Unbound public symbol ") + typeid(*this).name() + ": " + signature_string();
    }
    if (descriptor_ != nullptr) {
      return std::string("Unbound private symbol ") + typeid(*this).name() + ": " + descriptor_->to_string();
    }
    // NOTE(port): Object.toString's class@identity diagnostic uses the C++
    // dynamic type spelling and object address in place of JVM identityHashCode.
    std::ostringstream identity;
    identity << typeid(*this).name() << '@' << std::hex << reinterpret_cast<std::uintptr_t>(this);
    return "Unbound private symbol " + identity.str();
  }

 private:
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:56-59
  bool is_original_descriptor(const ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor& descriptor) const {
    // The upstream value-parameter original-descriptor issue is recorded in the
    // dependency audit; retain its actual containing-declaration recursion.
    const auto* parameter = dynamic_cast<const ::org::jetbrains::kotlin::descriptors::ValueParameterDescriptor*>(&descriptor);
    return (parameter != nullptr && is_original_descriptor(*parameter->get_containing_declaration())) ||
           descriptor == descriptor.get_original();
  }

  // NOTE(port): Kotlin's nullable string interpolation is explicit in C++.
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:37-37
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:68-68
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:75-75
  std::string signature_string() const {
    const auto* value = signature();
    return value != nullptr ? value->to_string() : "null";
  }

  Descriptor* const descriptor_;
  Owner* owner_ = nullptr;
  util::IdSignature* private_signature_ = nullptr;
};

template <typename Descriptor, typename Owner>
  requires std::derived_from<Descriptor, ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor> &&
           std::derived_from<Owner, declarations::IrSymbolOwner>
IrSymbolBase<Descriptor, Owner>::~IrSymbolBase() = default;

// Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:82-85
template <typename Descriptor, typename Owner>
  requires std::derived_from<Descriptor, ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor> &&
           std::derived_from<Owner, declarations::IrSymbolOwner>
class IrSymbolWithSignature : public IrSymbolBase<Descriptor, Owner> {
 public:
  virtual ~IrSymbolWithSignature() = 0;
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:82-85
  IrSymbolWithSignature(Descriptor* descriptor, util::IdSignature* signature)
      : IrSymbolBase<Descriptor, Owner>(descriptor), signature_(signature) {}
  // Transliterated from: compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:84-84
  util::IdSignature* signature() const override { return signature_; }

 private:
  util::IdSignature* const signature_;
};

template <typename Descriptor, typename Owner>
  requires std::derived_from<Descriptor, ::org::jetbrains::kotlin::descriptors::DeclarationDescriptor> &&
           std::derived_from<Owner, declarations::IrSymbolOwner>
IrSymbolWithSignature<Descriptor, Owner>::~IrSymbolWithSignature() = default;

}  // namespace org::jetbrains::kotlin::ir::symbols::impl
