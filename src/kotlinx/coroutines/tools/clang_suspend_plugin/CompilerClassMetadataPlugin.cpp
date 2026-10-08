/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/RTTIGenerator.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/RTTIGenerator.kt:198-269
// NOTE(port): Clang ABI binding for the consumed immutable class/name/subtype
// fields only. It operates on actual translated declarations, independently of
// compiler-object runtime bodies, so those bodies can themselves be compiled.
// This does not generate Native object layout, GC metadata or coroutine frames.

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ConvertUTF.h"

#include <map>
#include <memory>
#include <optional>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace {
using namespace clang;

struct SourceClass final {
  std::string package_name;
  std::string relative_name;
  bool is_interface;
  std::string identity() const { return package_name + ":" + relative_name; }
};

// NOTE(port): All annotations describe pinned internal source declarations;
// ordinary application classes are not required to carry these annotations.
class CompilerClassMetadataConsumer final : public ASTConsumer {
 public:
  explicit CompilerClassMetadataConsumer(CompilerInstance& compiler)
      : compiler_(compiler) {}

  void Initialize(ASTContext& context) override { context_ = &context; }

  void HandleTagDeclDefinition(TagDecl* tag) override {
    auto* record = dyn_cast<CXXRecordDecl>(tag);
    if (record == nullptr || record->isInvalidDecl()) return;
    for (const auto* attribute : record->specific_attrs<AnnotateAttr>()) {
      if (attribute->getAnnotation() == "kotlin.compiler.class_info") {
        info_record_ = record;
        return;
      }
    }
    const auto source = source_class(record);
    auto* inherited = find_accessor(record);
    if (!source) {
      // Abstract generic projections preserve the actual annotated source
      // bases. A concrete compiler object may not inherit somebody else's
      // identity merely because its own source mapping is missing.
      if (inherited != nullptr && !record->isAbstract() &&
          !record->isDependentType()) {
        error(record->getLocation(),
              "concrete translated compiler class lacks its Kotlin source identity");
      }
      return;
    }
    if (info_record_ == nullptr) {
      error(record->getLocation(), "compiler class metadata ABI is not declared");
      return;
    }
    auto* metadata = class_metadata(record);
    if (metadata == nullptr || inherited == nullptr) return;
    install_accessor(record, inherited, metadata);
  }

 private:
  CompilerInstance& compiler_;
  ASTContext* context_ = nullptr;
  CXXRecordDecl* info_record_ = nullptr;
  std::map<std::string, VarDecl*> metadata_;

  void error(SourceLocation location, llvm::StringRef message) {
    auto& diagnostics = compiler_.getDiagnostics();
    const auto id = diagnostics.getCustomDiagID(DiagnosticsEngine::Error, "%0");
    diagnostics.Report(location, id) << message;
  }

  std::optional<SourceClass> source_class(const CXXRecordDecl* record) {
    if (const auto* specialization = dyn_cast<ClassTemplateSpecializationDecl>(record)) {
      record = specialization->getSpecializedTemplate()->getTemplatedDecl();
    }
    for (const auto* attribute : record->specific_attrs<AnnotateAttr>()) {
      auto text = attribute->getAnnotation();
      if (!text.consume_front("kotlin.class:")) continue;
      const auto package_and_rest = text.split(':');
      const auto name_and_kind = package_and_rest.second.split(':');
      if (name_and_kind.first.empty() ||
          (name_and_kind.second != "class" && name_and_kind.second != "interface")) {
        error(record->getLocation(), "invalid Kotlin compiler class source annotation");
        return std::nullopt;
      }
      return SourceClass{package_and_rest.first.str(), name_and_kind.first.str(),
                         name_and_kind.second == "interface"};
    }
    return std::nullopt;
  }

  CXXRecordDecl* base_record(QualType type) {
    if (auto* record = type->getAsCXXRecordDecl()) return record->getDefinition();
    if (const auto* specialization = type->getAs<TemplateSpecializationType>()) {
      if (const auto* declaration = dyn_cast_or_null<ClassTemplateDecl>(
              specialization->getTemplateName().getAsTemplateDecl())) {
        return declaration->getTemplatedDecl()->getDefinition();
      }
    }
    return nullptr;
  }

  CXXMethodDecl* find_accessor(CXXRecordDecl* record) {
    std::set<const CXXRecordDecl*> visited;
    return find_accessor(record, visited);
  }

  // Clang dependent template bases can refer back to the same primary
  // declaration. Traverse each actual declaration once rather than unfolding
  // that cyclic representation as if it were a concrete inheritance chain.
  CXXMethodDecl* find_accessor(CXXRecordDecl* record,
                                std::set<const CXXRecordDecl*>& visited) {
    if (!visited.insert(record->getCanonicalDecl()).second) return nullptr;
    for (auto* method : record->methods()) {
      if (method->getNameAsString() == "__kxs_compiler_type_info") return method;
    }
    for (const auto& base : record->bases()) {
      if (auto* parent = base_record(base.getType())) {
        if (auto* accessor = find_accessor(parent, visited)) return accessor;
      }
    }
    return nullptr;
  }

  // NOTE(port): Source hierarchy traversal is bound to actual CXXRecordDecl
  // bases. Generic C++ projections do not introduce additional Kotlin classes.
  void collect_bases(CXXRecordDecl* record, CXXRecordDecl*& super_class,
                     std::map<std::string, CXXRecordDecl*>& interfaces) {
    for (const auto& base : record->bases()) {
      auto* parent = base_record(base.getType());
      if (parent == nullptr) {
        error(record->getLocation(), "translated class base has no declaration binding");
        continue;
      }
      const auto source = source_class(parent);
      if (source && source->is_interface) {
        interfaces.emplace(source->identity(), parent);
      } else if (source && super_class == nullptr) {
        super_class = parent;
      }
      collect_bases(parent, super_class, interfaces);
    }
  }

  std::string symbol_name(llvm::StringRef identity, llvm::StringRef suffix = {}) {
    constexpr char digits[] = "0123456789abcdef";
    std::string result = "__kxs_class_";
    for (unsigned char byte : identity.bytes()) {
      result += digits[byte >> 4];
      result += digits[byte & 15];
    }
    result += suffix;
    return result;
  }

  Expr* integer(std::uint64_t value, QualType type, SourceLocation location) {
    return IntegerLiteral::Create(*context_, llvm::APInt(context_->getTypeSize(type), value),
                                  type, location);
  }

  Expr* null_pointer(QualType type, SourceLocation location) {
    return ImplicitCastExpr::Create(*context_, type, CK_NullToPointer,
        new (*context_) CXXNullPtrLiteralExpr(context_->NullPtrTy, location),
        nullptr, VK_PRValue, FPOptionsOverride());
  }

  Expr* address(VarDecl* variable, SourceLocation location) {
    auto* reference = DeclRefExpr::Create(*context_, {}, {}, variable, false,
        location, variable->getType(), VK_LValue);
    return UnaryOperator::Create(*context_, reference, UO_AddrOf,
        context_->getPointerType(variable->getType()), VK_PRValue, OK_Ordinary,
        location, false, FPOptionsOverride());
  }

  VarDecl* global(llvm::StringRef name, QualType type, Expr* initializer,
                  SourceLocation location) {
    auto* variable = VarDecl::Create(*context_, context_->getTranslationUnitDecl(),
        location, location, &context_->Idents.get(name), type,
        context_->getTrivialTypeSourceInfo(type, location), SC_Extern);
    variable->addAttr(WeakAttr::CreateImplicit(*context_));
    variable->setInit(initializer);
    context_->getTranslationUnitDecl()->addDecl(variable);
    // Publish generated declarations to the same ordinary Clang code generator.
    compiler_.getASTConsumer().HandleTopLevelDecl(DeclGroupRef(variable));
    return variable;
  }

  std::pair<Expr*, std::size_t> name_literal(llvm::StringRef name,
                                           QualType pointer_type,
                                           SourceLocation location) {
    llvm::SmallVector<llvm::UTF16, 32> units;
    if (!llvm::convertUTF8ToUTF16String(name, units)) {
      error(location, "Kotlin source name is not valid UTF-8");
      return {nullptr, 0};
    }
    // Clang stores UTF-16 literal code units in target byte order.
    std::string bytes;
    for (const auto unit : units) {
      if (context_->getTargetInfo().isBigEndian()) {
        bytes += static_cast<char>(unit >> 8);
        bytes += static_cast<char>(unit);
      } else {
        bytes += static_cast<char>(unit);
        bytes += static_cast<char>(unit >> 8);
      }
    }
    auto* literal = StringLiteral::Create(*context_, bytes, StringLiteralKind::UTF16,
        false, context_->getStringLiteralArrayType(context_->Char16Ty, units.size()),
        {location});
    return {ImplicitCastExpr::Create(*context_, pointer_type, CK_ArrayToPointerDecay,
        literal, nullptr, VK_PRValue, FPOptionsOverride()), units.size()};
  }

  // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/RTTIGenerator.kt:198-269
  VarDecl* class_metadata(CXXRecordDecl* record) {
    const auto source = source_class(record);
    if (!source) return nullptr;
    const auto identity = source->identity();
    if (const auto entry = metadata_.find(identity); entry != metadata_.end()) {
      return entry->second;
    }
    CXXRecordDecl* super_class = nullptr;
    std::map<std::string, CXXRecordDecl*> interfaces;
    collect_bases(record, super_class, interfaces);
    llvm::SmallVector<FieldDecl*, 8> fields(info_record_->fields());
    if (fields.size() != 8) {
      error(record->getLocation(), "compiler class metadata ABI field mismatch");
      return nullptr;
    }
    const auto location = record->getLocation();
    Expr* super = null_pointer(fields[0]->getType(), location);
    if (super_class != nullptr) {
      auto* parent = class_metadata(super_class);
      if (parent == nullptr) return nullptr;
      super = address(parent, location);
    }
    Expr* interface_pointer = null_pointer(fields[1]->getType(), location);
    if (!interfaces.empty()) {
      llvm::SmallVector<Expr*, 8> values;
      for (const auto& entry : interfaces) {
        auto* metadata = class_metadata(entry.second);
        if (metadata == nullptr) return nullptr;
        values.push_back(address(metadata, location));
      }
      auto array_type = context_->getConstantArrayType(
          fields[1]->getType()->getPointeeType(), llvm::APInt(64, values.size()),
          nullptr, ArraySizeModifier::Normal, 0);
      auto* list = new (*context_) InitListExpr(*context_, location, values, location, false);
      list->setType(array_type);
      auto* array = global(symbol_name(identity, "_interfaces"), array_type, list, location);
      auto* reference = DeclRefExpr::Create(*context_, {}, {}, array, false,
          location, array_type, VK_LValue);
      interface_pointer = ImplicitCastExpr::Create(*context_, fields[1]->getType(),
          CK_ArrayToPointerDecay, reference, nullptr, VK_PRValue, FPOptionsOverride());
    }
    // The translated declarations currently bound here are named, nonlocal
    // source classes/interfaces. Preserve their exact package and relative name.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/RTTIGenerator.kt:583-613
    auto package = name_literal(source->package_name, fields[3]->getType(), location);
    auto relative = name_literal(source->relative_name, fields[5]->getType(), location);
    if (package.first == nullptr || relative.first == nullptr) return nullptr;
    llvm::SmallVector<Expr*, 8> values{
        super, interface_pointer, integer(interfaces.size(), fields[2]->getType(), location),
        package.first, integer(package.second, fields[4]->getType(), location),
        relative.first, integer(relative.second, fields[6]->getType(), location),
        integer(256 | 512 | (source->is_interface ? 4 : 0), fields[7]->getType(), location)};
    const auto type = context_->getTypeDeclType(static_cast<const TypeDecl*>(info_record_)).withConst();
    auto* initializer = new (*context_) InitListExpr(*context_, location, values, location, false);
    initializer->setType(type);
    auto* result = global(symbol_name(identity), type, initializer, location);
    metadata_.emplace(identity, result);
    return result;
  }

  // NOTE(port): Bind source ObjHeader::type_info retrieval to the actual C++
  // dynamic declaration. Overrides reuse the root virtual slot; no Native
  // layout or application class transformation is involved.
  void install_accessor(CXXRecordDecl* record, CXXMethodDecl* inherited,
                         VarDecl* metadata) {
    auto* method = inherited;
    const auto location = record->getLocation();
    if (inherited->getParent() != record) {
      method = CXXMethodDecl::Create(*context_, record, location,
          DeclarationNameInfo(inherited->getDeclName(), location), inherited->getType(),
          context_->getTrivialTypeSourceInfo(inherited->getType(), location),
          SC_None, false, true, ConstexprSpecKind::Unspecified, location);
      method->setAccess(AS_private);
      method->setImplicit();
      method->addOverriddenMethod(inherited);
      record->addDecl(method);
    }
    if (method->hasBody()) return;
    method->setInlineSpecified(true);
    auto* statement = ReturnStmt::Create(*context_, location, address(metadata, location), nullptr);
    method->setBody(CompoundStmt::Create(*context_, {statement}, FPOptionsOverride(),
                                         location, location));
  }
};

class CompilerClassMetadataAction final : public PluginASTAction {
 protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& compiler,
                                                llvm::StringRef) override {
    return std::make_unique<CompilerClassMetadataConsumer>(compiler);
  }
  bool ParseArgs(const CompilerInstance&, const std::vector<std::string>& args) override {
    return args.empty();
  }
  ActionType getActionType() override { return AddBeforeMainAction; }
};
FrontendPluginRegistry::Add<CompilerClassMetadataAction> registration(
    "kotlin-compiler-class-metadata", "Bind translated compiler class metadata to C++ declarations");
}  // namespace
