/*
 * Copyright 2010-2015 JetBrains s.r.o. Use of this source code is governed by the Apache 2.0 license.
 */
// port-lint: source core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:27-70
#include "DeclarationDescriptorImpl.hpp"
#include "../../renderer/DescriptorRenderer.hpp"

#include <sstream>
#include <typeinfo>
#include <utility>

namespace org::jetbrains::kotlin::descriptors::impl {
namespace {
// NOTE(port): The inherited compiler diagnostic API is a byte string. Encode
// source UTF-16 as UTF-8/WTF-8 without changing Name storage or comparison.
std::string diagnostic_string(const std::u16string& text) {
  std::string result;
  for (std::size_t i = 0; i < text.size(); ++i) {
    std::uint32_t ch = text[i];
    if (ch >= 0xd800 && ch <= 0xdbff && i + 1 < text.size() &&
        text[i + 1] >= 0xdc00 && text[i + 1] <= 0xdfff) {
      ch = 0x10000 + ((ch - 0xd800) << 10) + (text[++i] - 0xdc00);
    }
    if (ch < 0x80) {
      result.push_back(static_cast<char>(ch));
    } else if (ch < 0x800) {
      result.push_back(static_cast<char>(0xc0 | (ch >> 6)));
      result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
    } else if (ch < 0x10000) {
      result.push_back(static_cast<char>(0xe0 | (ch >> 12)));
      result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
    } else {
      result.push_back(static_cast<char>(0xf0 | (ch >> 18)));
      result.push_back(static_cast<char>(0x80 | ((ch >> 12) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (ch & 0x3f)));
    }
  }
  return result;
}
}  // namespace

// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:32-35
DeclarationDescriptorImpl::DeclarationDescriptorImpl(
    const annotations::Annotations& annotations, name::Name name)
    : AnnotatedImpl(annotations), name_(std::move(name)) {}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:37-41
const name::Name& DeclarationDescriptorImpl::get_name() const { return name_; }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:43-47
DeclarationDescriptor& DeclarationDescriptorImpl::get_original() const {
  return const_cast<DeclarationDescriptorImpl&>(*this);
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:49-52
void DeclarationDescriptorImpl::accept_void(DeclarationDescriptorVisitor<void, std::nullptr_t>& visitor) {
  accept(visitor, nullptr);
}
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:54-57
std::string DeclarationDescriptorImpl::to_string() const { return to_string(*this); }
// Transliterated from: core/descriptors/src/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.java:59-69
std::string DeclarationDescriptorImpl::to_string(const DeclarationDescriptor& descriptor) {
  try {
    auto rendered = diagnostic_string(renderer::DescriptorRenderer::DEBUG_TEXT.render(descriptor));
    // NOTE(port): Compiler-object RTTI/address identity supplies the existing
    // C++ diagnostic boundary. Exact Java simple-name/hash spelling is unverified;
    // neither these addresses nor RTTI describe Native runtime object layout.
    std::ostringstream suffix;
    suffix << '[' << typeid(descriptor).name() << '@' << std::hex
           << static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&descriptor)) << ']';
    return rendered + suffix.str();
  } catch (...) {
    // DescriptionRenderer may throw if this is not yet completely initialized
    // It is very inconvenient while debugging
    return std::string(typeid(descriptor).name()) + " " + diagnostic_string(descriptor.get_name().to_string());
  }
}
}  // namespace org::jetbrains::kotlin::descriptors::impl
