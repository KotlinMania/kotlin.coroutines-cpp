/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:18-285
#pragma once

#include "IrVisitor.hpp"
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

// NOTE(port): These forward declarations name real IR classes. Their full
// hierarchy must be translated before the virtual defaults can instantiate.
namespace org::jetbrains::kotlin::ir { class IrElement; }
namespace org::jetbrains::kotlin::ir::symbols { class IrSymbol; }
namespace org::jetbrains::kotlin::ir::declarations {
class IrDeclarationBase;
class IrValueParameter;
class IrClass;
class IrAnonymousInitializer;
class IrTypeParameter;
class IrFunction;
class IrConstructor;
class IrEnumEntry;
class IrField;
class IrLocalDelegatedProperty;
class IrModuleFragment;
class IrProperty;
class IrScript;
class IrReplSnippet;
class IrSimpleFunction;
class IrTypeAlias;
class IrVariable;
class IrPackageFragment;
class IrExternalPackageFragment;
class IrFile;
}
namespace org::jetbrains::kotlin::ir::expressions {
class IrExpression;
class IrBody;
class IrExpressionBody;
class IrBlockBody;
class IrDeclarationReference;
template <typename Symbol> class IrMemberAccessExpression;
class IrFunctionAccessExpression;
class IrConstructorCall;
class IrAnnotation;
class IrGetSingletonValue;
class IrGetObjectValue;
class IrGetEnumValue;
class IrRawFunctionReference;
class IrContainerExpression;
class IrBlock;
class IrComposite;
class IrReturnableBlock;
class IrInlinedFunctionBlock;
class IrSyntheticBody;
class IrBreakContinue;
class IrBreak;
class IrContinue;
class IrCall;
template <typename Symbol> class IrCallableReference;
class IrFunctionReference;
class IrPropertyReference;
class IrLocalDelegatedPropertyReference;
template <typename Symbol> class IrRichCallableReference;
class IrRichFunctionReference;
class IrRichPropertyReference;
class IrClassReference;
class IrConst;
class IrConstantValue;
class IrConstantPrimitive;
class IrConstantObject;
class IrConstantArray;
class IrDelegatingConstructorCall;
class IrDynamicExpression;
class IrDynamicOperatorExpression;
class IrDynamicMemberExpression;
class IrEnumConstructorCall;
class IrErrorExpression;
class IrErrorCallExpression;
class IrFieldAccessExpression;
class IrGetField;
class IrSetField;
class IrFunctionExpression;
class IrGetClass;
class IrInstanceInitializerCall;
class IrLoop;
class IrWhileLoop;
class IrDoWhileLoop;
class IrReturn;
class IrStringConcatenation;
class IrSuspensionPoint;
class IrSuspendableExpression;
class IrThrow;
class IrTry;
class IrCatch;
class IrTypeOperatorCall;
class IrValueAccessExpression;
class IrGetValue;
class IrSetValue;
class IrVararg;
class IrSpreadElement;
class IrWhen;
class IrBranch;
class IrElseBranch;
}


namespace org::jetbrains::kotlin::ir::visitors {
template <typename D> class IrTransformer;
namespace detail {
// NOTE(port): C++ cannot declare virtual function templates. This boundary
// retains the actual node reference, generic context and result identity.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
class IrVisitorDispatch {
 public:
  virtual ~IrVisitorDispatch() = default;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
  void accept(IrElement& element);
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:20-20
  virtual void visit_element(::org::jetbrains::kotlin::ir::IrElement& element) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:22-23
  virtual void visit_declaration(declarations::IrDeclarationBase& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:25-26
  virtual void visit_value_parameter(declarations::IrValueParameter& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:28-29
  virtual void visit_class(declarations::IrClass& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:31-32
  virtual void visit_anonymous_initializer(declarations::IrAnonymousInitializer& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:34-35
  virtual void visit_type_parameter(declarations::IrTypeParameter& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:37-38
  virtual void visit_function(declarations::IrFunction& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:40-41
  virtual void visit_constructor(declarations::IrConstructor& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:43-44
  virtual void visit_enum_entry(declarations::IrEnumEntry& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:46-47
  virtual void visit_field(declarations::IrField& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:49-50
  virtual void visit_local_delegated_property(declarations::IrLocalDelegatedProperty& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:52-53
  virtual void visit_module_fragment(declarations::IrModuleFragment& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:55-56
  virtual void visit_property(declarations::IrProperty& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:58-59
  virtual void visit_script(declarations::IrScript& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:61-62
  virtual void visit_repl_snippet(declarations::IrReplSnippet& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:64-65
  virtual void visit_simple_function(declarations::IrSimpleFunction& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:67-68
  virtual void visit_type_alias(declarations::IrTypeAlias& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:70-71
  virtual void visit_variable(declarations::IrVariable& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:73-74
  virtual void visit_package_fragment(declarations::IrPackageFragment& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:76-77
  virtual void visit_external_package_fragment(declarations::IrExternalPackageFragment& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:79-80
  virtual void visit_file(declarations::IrFile& declaration) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:82-83
  virtual void visit_expression(expressions::IrExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:85-86
  virtual void visit_body(expressions::IrBody& body) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:88-89
  virtual void visit_expression_body(expressions::IrExpressionBody& body) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:91-92
  virtual void visit_block_body(expressions::IrBlockBody& body) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:94-95
  virtual void visit_declaration_reference(expressions::IrDeclarationReference& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:97-98
  virtual void visit_member_access(expressions::IrMemberAccessExpression<symbols::IrSymbol>& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:100-101
  virtual void visit_function_access(expressions::IrFunctionAccessExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:103-104
  virtual void visit_constructor_call(expressions::IrConstructorCall& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:106-107
  virtual void visit_annotation(expressions::IrAnnotation& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:109-110
  virtual void visit_singleton_reference(expressions::IrGetSingletonValue& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:112-113
  virtual void visit_get_object_value(expressions::IrGetObjectValue& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:115-116
  virtual void visit_get_enum_value(expressions::IrGetEnumValue& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:118-119
  virtual void visit_raw_function_reference(expressions::IrRawFunctionReference& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:121-122
  virtual void visit_container_expression(expressions::IrContainerExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:124-125
  virtual void visit_block(expressions::IrBlock& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:127-128
  virtual void visit_composite(expressions::IrComposite& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:130-131
  virtual void visit_returnable_block(expressions::IrReturnableBlock& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:133-134
  virtual void visit_inlined_function_block(expressions::IrInlinedFunctionBlock& inlined_block) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:136-137
  virtual void visit_synthetic_body(expressions::IrSyntheticBody& body) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:139-140
  virtual void visit_break_continue(expressions::IrBreakContinue& jump) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:142-143
  virtual void visit_break(expressions::IrBreak& jump) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:145-146
  virtual void visit_continue(expressions::IrContinue& jump) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:148-149
  virtual void visit_call(expressions::IrCall& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:151-152
  virtual void visit_callable_reference(expressions::IrCallableReference<symbols::IrSymbol>& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:154-155
  virtual void visit_function_reference(expressions::IrFunctionReference& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:157-158
  virtual void visit_property_reference(expressions::IrPropertyReference& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:160-161
  virtual void visit_local_delegated_property_reference(expressions::IrLocalDelegatedPropertyReference& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:163-164
  virtual void visit_rich_callable_reference(expressions::IrRichCallableReference<symbols::IrSymbol>& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:166-167
  virtual void visit_rich_function_reference(expressions::IrRichFunctionReference& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:169-170
  virtual void visit_rich_property_reference(expressions::IrRichPropertyReference& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:172-173
  virtual void visit_class_reference(expressions::IrClassReference& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:175-176
  virtual void visit_const(expressions::IrConst& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:178-179
  virtual void visit_constant_value(expressions::IrConstantValue& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:181-182
  virtual void visit_constant_primitive(expressions::IrConstantPrimitive& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:184-185
  virtual void visit_constant_object(expressions::IrConstantObject& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:187-188
  virtual void visit_constant_array(expressions::IrConstantArray& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:190-191
  virtual void visit_delegating_constructor_call(expressions::IrDelegatingConstructorCall& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:193-194
  virtual void visit_dynamic_expression(expressions::IrDynamicExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:196-197
  virtual void visit_dynamic_operator_expression(expressions::IrDynamicOperatorExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:199-200
  virtual void visit_dynamic_member_expression(expressions::IrDynamicMemberExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:202-203
  virtual void visit_enum_constructor_call(expressions::IrEnumConstructorCall& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:205-206
  virtual void visit_error_expression(expressions::IrErrorExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:208-209
  virtual void visit_error_call_expression(expressions::IrErrorCallExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:211-212
  virtual void visit_field_access(expressions::IrFieldAccessExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:214-215
  virtual void visit_get_field(expressions::IrGetField& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:217-218
  virtual void visit_set_field(expressions::IrSetField& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:220-221
  virtual void visit_function_expression(expressions::IrFunctionExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:223-224
  virtual void visit_get_class(expressions::IrGetClass& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:226-227
  virtual void visit_instance_initializer_call(expressions::IrInstanceInitializerCall& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:229-230
  virtual void visit_loop(expressions::IrLoop& loop) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:232-233
  virtual void visit_while_loop(expressions::IrWhileLoop& loop) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:235-236
  virtual void visit_do_while_loop(expressions::IrDoWhileLoop& loop) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:238-239
  virtual void visit_return(expressions::IrReturn& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:241-242
  virtual void visit_string_concatenation(expressions::IrStringConcatenation& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:244-245
  virtual void visit_suspension_point(expressions::IrSuspensionPoint& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:247-248
  virtual void visit_suspendable_expression(expressions::IrSuspendableExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:250-251
  virtual void visit_throw(expressions::IrThrow& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:253-254
  virtual void visit_try(expressions::IrTry& a_try) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:256-257
  virtual void visit_catch(expressions::IrCatch& a_catch) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:259-260
  virtual void visit_type_operator(expressions::IrTypeOperatorCall& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:262-263
  virtual void visit_value_access(expressions::IrValueAccessExpression& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:265-266
  virtual void visit_get_value(expressions::IrGetValue& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:268-269
  virtual void visit_set_value(expressions::IrSetValue& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:271-272
  virtual void visit_vararg(expressions::IrVararg& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:274-275
  virtual void visit_spread_element(expressions::IrSpreadElement& spread) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:277-278
  virtual void visit_when(expressions::IrWhen& expression) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:280-281
  virtual void visit_branch(expressions::IrBranch& branch) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:283-284
  virtual void visit_else_branch(expressions::IrElseBranch& branch) = 0;
};
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62,86-86
class IrTransformerDispatch : public virtual IrVisitorDispatch {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62
  IrElement& transform(IrElement& element);
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:86-86
  void transform_children(IrElement& element);
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62
  virtual IrElement& take_element_result() = 0;
};
// NOTE(port): Store a real result without requiring default construction or
// copying. Reference results retain their original object; Unit maps to void.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
template <typename R>
class IrVisitorResult {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
  void set(R value) {
    if constexpr (std::is_reference_v<R>) value_.emplace(std::ref(value));
    else value_.emplace(std::move(value));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
  R take() {
    if constexpr (std::is_reference_v<R>) return value_->get();
    else return std::move(*value_);
  }
 private:
  using Stored = std::conditional_t<std::is_reference_v<R>,
      std::reference_wrapper<std::remove_reference_t<R>>, R>;
  std::optional<Stored> value_;
};
// NOTE(port): Unit has no C++ result object to retrieve.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53,74-74
template <> class IrVisitorResult<void> {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53,74-74
  void take() {}
};
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
template <typename R, typename D>
class TypedIrVisitorDispatch : public virtual IrVisitorDispatch {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
  TypedIrVisitorDispatch(IrVisitor<R, D>& visitor, D& data) : visitor_(visitor), data_(data) {}
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
  R take_result() { return result_.take(); }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:20-20
  void visit_element(::org::jetbrains::kotlin::ir::IrElement& element) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_element(element, data_);
    else result_.set(visitor_.visit_element(element, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:22-23
  void visit_declaration(declarations::IrDeclarationBase& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_declaration(declaration, data_);
    else result_.set(visitor_.visit_declaration(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:25-26
  void visit_value_parameter(declarations::IrValueParameter& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_value_parameter(declaration, data_);
    else result_.set(visitor_.visit_value_parameter(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:28-29
  void visit_class(declarations::IrClass& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_class(declaration, data_);
    else result_.set(visitor_.visit_class(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:31-32
  void visit_anonymous_initializer(declarations::IrAnonymousInitializer& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_anonymous_initializer(declaration, data_);
    else result_.set(visitor_.visit_anonymous_initializer(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:34-35
  void visit_type_parameter(declarations::IrTypeParameter& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_type_parameter(declaration, data_);
    else result_.set(visitor_.visit_type_parameter(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:37-38
  void visit_function(declarations::IrFunction& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_function(declaration, data_);
    else result_.set(visitor_.visit_function(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:40-41
  void visit_constructor(declarations::IrConstructor& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_constructor(declaration, data_);
    else result_.set(visitor_.visit_constructor(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:43-44
  void visit_enum_entry(declarations::IrEnumEntry& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_enum_entry(declaration, data_);
    else result_.set(visitor_.visit_enum_entry(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:46-47
  void visit_field(declarations::IrField& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_field(declaration, data_);
    else result_.set(visitor_.visit_field(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:49-50
  void visit_local_delegated_property(declarations::IrLocalDelegatedProperty& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_local_delegated_property(declaration, data_);
    else result_.set(visitor_.visit_local_delegated_property(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:52-53
  void visit_module_fragment(declarations::IrModuleFragment& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_module_fragment(declaration, data_);
    else result_.set(visitor_.visit_module_fragment(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:55-56
  void visit_property(declarations::IrProperty& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_property(declaration, data_);
    else result_.set(visitor_.visit_property(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:58-59
  void visit_script(declarations::IrScript& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_script(declaration, data_);
    else result_.set(visitor_.visit_script(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:61-62
  void visit_repl_snippet(declarations::IrReplSnippet& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_repl_snippet(declaration, data_);
    else result_.set(visitor_.visit_repl_snippet(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:64-65
  void visit_simple_function(declarations::IrSimpleFunction& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_simple_function(declaration, data_);
    else result_.set(visitor_.visit_simple_function(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:67-68
  void visit_type_alias(declarations::IrTypeAlias& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_type_alias(declaration, data_);
    else result_.set(visitor_.visit_type_alias(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:70-71
  void visit_variable(declarations::IrVariable& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_variable(declaration, data_);
    else result_.set(visitor_.visit_variable(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:73-74
  void visit_package_fragment(declarations::IrPackageFragment& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_package_fragment(declaration, data_);
    else result_.set(visitor_.visit_package_fragment(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:76-77
  void visit_external_package_fragment(declarations::IrExternalPackageFragment& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_external_package_fragment(declaration, data_);
    else result_.set(visitor_.visit_external_package_fragment(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:79-80
  void visit_file(declarations::IrFile& declaration) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_file(declaration, data_);
    else result_.set(visitor_.visit_file(declaration, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:82-83
  void visit_expression(expressions::IrExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_expression(expression, data_);
    else result_.set(visitor_.visit_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:85-86
  void visit_body(expressions::IrBody& body) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_body(body, data_);
    else result_.set(visitor_.visit_body(body, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:88-89
  void visit_expression_body(expressions::IrExpressionBody& body) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_expression_body(body, data_);
    else result_.set(visitor_.visit_expression_body(body, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:91-92
  void visit_block_body(expressions::IrBlockBody& body) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_block_body(body, data_);
    else result_.set(visitor_.visit_block_body(body, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:94-95
  void visit_declaration_reference(expressions::IrDeclarationReference& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_declaration_reference(expression, data_);
    else result_.set(visitor_.visit_declaration_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:97-98
  void visit_member_access(expressions::IrMemberAccessExpression<symbols::IrSymbol>& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_member_access(expression, data_);
    else result_.set(visitor_.visit_member_access(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:100-101
  void visit_function_access(expressions::IrFunctionAccessExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_function_access(expression, data_);
    else result_.set(visitor_.visit_function_access(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:103-104
  void visit_constructor_call(expressions::IrConstructorCall& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_constructor_call(expression, data_);
    else result_.set(visitor_.visit_constructor_call(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:106-107
  void visit_annotation(expressions::IrAnnotation& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_annotation(expression, data_);
    else result_.set(visitor_.visit_annotation(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:109-110
  void visit_singleton_reference(expressions::IrGetSingletonValue& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_singleton_reference(expression, data_);
    else result_.set(visitor_.visit_singleton_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:112-113
  void visit_get_object_value(expressions::IrGetObjectValue& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_get_object_value(expression, data_);
    else result_.set(visitor_.visit_get_object_value(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:115-116
  void visit_get_enum_value(expressions::IrGetEnumValue& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_get_enum_value(expression, data_);
    else result_.set(visitor_.visit_get_enum_value(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:118-119
  void visit_raw_function_reference(expressions::IrRawFunctionReference& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_raw_function_reference(expression, data_);
    else result_.set(visitor_.visit_raw_function_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:121-122
  void visit_container_expression(expressions::IrContainerExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_container_expression(expression, data_);
    else result_.set(visitor_.visit_container_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:124-125
  void visit_block(expressions::IrBlock& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_block(expression, data_);
    else result_.set(visitor_.visit_block(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:127-128
  void visit_composite(expressions::IrComposite& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_composite(expression, data_);
    else result_.set(visitor_.visit_composite(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:130-131
  void visit_returnable_block(expressions::IrReturnableBlock& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_returnable_block(expression, data_);
    else result_.set(visitor_.visit_returnable_block(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:133-134
  void visit_inlined_function_block(expressions::IrInlinedFunctionBlock& inlined_block) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_inlined_function_block(inlined_block, data_);
    else result_.set(visitor_.visit_inlined_function_block(inlined_block, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:136-137
  void visit_synthetic_body(expressions::IrSyntheticBody& body) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_synthetic_body(body, data_);
    else result_.set(visitor_.visit_synthetic_body(body, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:139-140
  void visit_break_continue(expressions::IrBreakContinue& jump) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_break_continue(jump, data_);
    else result_.set(visitor_.visit_break_continue(jump, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:142-143
  void visit_break(expressions::IrBreak& jump) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_break(jump, data_);
    else result_.set(visitor_.visit_break(jump, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:145-146
  void visit_continue(expressions::IrContinue& jump) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_continue(jump, data_);
    else result_.set(visitor_.visit_continue(jump, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:148-149
  void visit_call(expressions::IrCall& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_call(expression, data_);
    else result_.set(visitor_.visit_call(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:151-152
  void visit_callable_reference(expressions::IrCallableReference<symbols::IrSymbol>& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_callable_reference(expression, data_);
    else result_.set(visitor_.visit_callable_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:154-155
  void visit_function_reference(expressions::IrFunctionReference& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_function_reference(expression, data_);
    else result_.set(visitor_.visit_function_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:157-158
  void visit_property_reference(expressions::IrPropertyReference& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_property_reference(expression, data_);
    else result_.set(visitor_.visit_property_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:160-161
  void visit_local_delegated_property_reference(expressions::IrLocalDelegatedPropertyReference& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_local_delegated_property_reference(expression, data_);
    else result_.set(visitor_.visit_local_delegated_property_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:163-164
  void visit_rich_callable_reference(expressions::IrRichCallableReference<symbols::IrSymbol>& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_rich_callable_reference(expression, data_);
    else result_.set(visitor_.visit_rich_callable_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:166-167
  void visit_rich_function_reference(expressions::IrRichFunctionReference& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_rich_function_reference(expression, data_);
    else result_.set(visitor_.visit_rich_function_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:169-170
  void visit_rich_property_reference(expressions::IrRichPropertyReference& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_rich_property_reference(expression, data_);
    else result_.set(visitor_.visit_rich_property_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:172-173
  void visit_class_reference(expressions::IrClassReference& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_class_reference(expression, data_);
    else result_.set(visitor_.visit_class_reference(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:175-176
  void visit_const(expressions::IrConst& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_const(expression, data_);
    else result_.set(visitor_.visit_const(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:178-179
  void visit_constant_value(expressions::IrConstantValue& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_constant_value(expression, data_);
    else result_.set(visitor_.visit_constant_value(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:181-182
  void visit_constant_primitive(expressions::IrConstantPrimitive& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_constant_primitive(expression, data_);
    else result_.set(visitor_.visit_constant_primitive(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:184-185
  void visit_constant_object(expressions::IrConstantObject& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_constant_object(expression, data_);
    else result_.set(visitor_.visit_constant_object(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:187-188
  void visit_constant_array(expressions::IrConstantArray& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_constant_array(expression, data_);
    else result_.set(visitor_.visit_constant_array(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:190-191
  void visit_delegating_constructor_call(expressions::IrDelegatingConstructorCall& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_delegating_constructor_call(expression, data_);
    else result_.set(visitor_.visit_delegating_constructor_call(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:193-194
  void visit_dynamic_expression(expressions::IrDynamicExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_dynamic_expression(expression, data_);
    else result_.set(visitor_.visit_dynamic_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:196-197
  void visit_dynamic_operator_expression(expressions::IrDynamicOperatorExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_dynamic_operator_expression(expression, data_);
    else result_.set(visitor_.visit_dynamic_operator_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:199-200
  void visit_dynamic_member_expression(expressions::IrDynamicMemberExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_dynamic_member_expression(expression, data_);
    else result_.set(visitor_.visit_dynamic_member_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:202-203
  void visit_enum_constructor_call(expressions::IrEnumConstructorCall& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_enum_constructor_call(expression, data_);
    else result_.set(visitor_.visit_enum_constructor_call(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:205-206
  void visit_error_expression(expressions::IrErrorExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_error_expression(expression, data_);
    else result_.set(visitor_.visit_error_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:208-209
  void visit_error_call_expression(expressions::IrErrorCallExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_error_call_expression(expression, data_);
    else result_.set(visitor_.visit_error_call_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:211-212
  void visit_field_access(expressions::IrFieldAccessExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_field_access(expression, data_);
    else result_.set(visitor_.visit_field_access(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:214-215
  void visit_get_field(expressions::IrGetField& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_get_field(expression, data_);
    else result_.set(visitor_.visit_get_field(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:217-218
  void visit_set_field(expressions::IrSetField& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_set_field(expression, data_);
    else result_.set(visitor_.visit_set_field(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:220-221
  void visit_function_expression(expressions::IrFunctionExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_function_expression(expression, data_);
    else result_.set(visitor_.visit_function_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:223-224
  void visit_get_class(expressions::IrGetClass& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_get_class(expression, data_);
    else result_.set(visitor_.visit_get_class(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:226-227
  void visit_instance_initializer_call(expressions::IrInstanceInitializerCall& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_instance_initializer_call(expression, data_);
    else result_.set(visitor_.visit_instance_initializer_call(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:229-230
  void visit_loop(expressions::IrLoop& loop) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_loop(loop, data_);
    else result_.set(visitor_.visit_loop(loop, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:232-233
  void visit_while_loop(expressions::IrWhileLoop& loop) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_while_loop(loop, data_);
    else result_.set(visitor_.visit_while_loop(loop, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:235-236
  void visit_do_while_loop(expressions::IrDoWhileLoop& loop) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_do_while_loop(loop, data_);
    else result_.set(visitor_.visit_do_while_loop(loop, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:238-239
  void visit_return(expressions::IrReturn& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_return(expression, data_);
    else result_.set(visitor_.visit_return(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:241-242
  void visit_string_concatenation(expressions::IrStringConcatenation& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_string_concatenation(expression, data_);
    else result_.set(visitor_.visit_string_concatenation(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:244-245
  void visit_suspension_point(expressions::IrSuspensionPoint& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_suspension_point(expression, data_);
    else result_.set(visitor_.visit_suspension_point(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:247-248
  void visit_suspendable_expression(expressions::IrSuspendableExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_suspendable_expression(expression, data_);
    else result_.set(visitor_.visit_suspendable_expression(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:250-251
  void visit_throw(expressions::IrThrow& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_throw(expression, data_);
    else result_.set(visitor_.visit_throw(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:253-254
  void visit_try(expressions::IrTry& a_try) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_try(a_try, data_);
    else result_.set(visitor_.visit_try(a_try, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:256-257
  void visit_catch(expressions::IrCatch& a_catch) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_catch(a_catch, data_);
    else result_.set(visitor_.visit_catch(a_catch, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:259-260
  void visit_type_operator(expressions::IrTypeOperatorCall& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_type_operator(expression, data_);
    else result_.set(visitor_.visit_type_operator(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:262-263
  void visit_value_access(expressions::IrValueAccessExpression& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_value_access(expression, data_);
    else result_.set(visitor_.visit_value_access(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:265-266
  void visit_get_value(expressions::IrGetValue& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_get_value(expression, data_);
    else result_.set(visitor_.visit_get_value(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:268-269
  void visit_set_value(expressions::IrSetValue& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_set_value(expression, data_);
    else result_.set(visitor_.visit_set_value(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:271-272
  void visit_vararg(expressions::IrVararg& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_vararg(expression, data_);
    else result_.set(visitor_.visit_vararg(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:274-275
  void visit_spread_element(expressions::IrSpreadElement& spread) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_spread_element(spread, data_);
    else result_.set(visitor_.visit_spread_element(spread, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:277-278
  void visit_when(expressions::IrWhen& expression) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_when(expression, data_);
    else result_.set(visitor_.visit_when(expression, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:280-281
  void visit_branch(expressions::IrBranch& branch) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_branch(branch, data_);
    else result_.set(visitor_.visit_branch(branch, data_));
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:283-284
  void visit_else_branch(expressions::IrElseBranch& branch) override {
    if constexpr (std::is_void_v<R>) visitor_.visit_else_branch(branch, data_);
    else result_.set(visitor_.visit_else_branch(branch, data_));
  }
 private:
  IrVisitor<R, D>& visitor_;
  D& data_;
  IrVisitorResult<R> result_;
};
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62,86-86
template <typename D>
class TypedIrTransformerDispatch final : public TypedIrVisitorDispatch<IrElement&, D>, public IrTransformerDispatch {
 public:
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62,86-86
  TypedIrTransformerDispatch(IrTransformer<D>& transformer, D& data)
      : TypedIrVisitorDispatch<IrElement&, D>(transformer, data) {}
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62
  IrElement& take_element_result() override { return this->take_result(); }
};
}  // namespace detail
}  // namespace org::jetbrains::kotlin::ir::visitors
