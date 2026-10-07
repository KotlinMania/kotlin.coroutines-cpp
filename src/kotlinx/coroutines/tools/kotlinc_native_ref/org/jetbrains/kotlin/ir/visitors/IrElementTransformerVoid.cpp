/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:1-590
#include "IrElementTransformerVoid.hpp"

namespace org::jetbrains::kotlin::ir::visitors {
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:27-29
void IrElementTransformerVoid::transform_children_void(IrElement& element) {
  visitors::transform_children_void(element, *this);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:31-34
IrElement& IrElementTransformerVoid::visit_element(IrElement& element) {
  element.transform_children(*this, nullptr);
  return element;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:36-37
IrElement& IrElementTransformerVoid::visit_element(IrElement& element, std::nullptr_t) {
  return visit_element(element);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:39-42
IrStatement& IrElementTransformerVoid::visit_declaration(declarations::IrDeclarationBase& declaration) {
  declaration.transform_children(*this, nullptr);
  return declaration;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:44-45
IrStatement& IrElementTransformerVoid::visit_declaration(declarations::IrDeclarationBase& declaration, std::nullptr_t) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:47-48
IrStatement& IrElementTransformerVoid::visit_value_parameter(declarations::IrValueParameter& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:50-51
IrStatement& IrElementTransformerVoid::visit_value_parameter(declarations::IrValueParameter& declaration, std::nullptr_t) {
  return visit_value_parameter(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:53-54
IrStatement& IrElementTransformerVoid::visit_class(declarations::IrClass& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:56-57
IrStatement& IrElementTransformerVoid::visit_class(declarations::IrClass& declaration, std::nullptr_t) {
  return visit_class(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:59-60
IrStatement& IrElementTransformerVoid::visit_anonymous_initializer(declarations::IrAnonymousInitializer& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:62-63
IrStatement& IrElementTransformerVoid::visit_anonymous_initializer(declarations::IrAnonymousInitializer& declaration, std::nullptr_t) {
  return visit_anonymous_initializer(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:65-66
IrStatement& IrElementTransformerVoid::visit_type_parameter(declarations::IrTypeParameter& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:68-69
IrStatement& IrElementTransformerVoid::visit_type_parameter(declarations::IrTypeParameter& declaration, std::nullptr_t) {
  return visit_type_parameter(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:71-72
IrStatement& IrElementTransformerVoid::visit_function(declarations::IrFunction& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:74-75
IrStatement& IrElementTransformerVoid::visit_function(declarations::IrFunction& declaration, std::nullptr_t) {
  return visit_function(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:77-78
IrStatement& IrElementTransformerVoid::visit_constructor(declarations::IrConstructor& declaration) {
  return visit_function(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:80-81
IrStatement& IrElementTransformerVoid::visit_constructor(declarations::IrConstructor& declaration, std::nullptr_t) {
  return visit_constructor(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:83-84
IrStatement& IrElementTransformerVoid::visit_enum_entry(declarations::IrEnumEntry& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:86-87
IrStatement& IrElementTransformerVoid::visit_enum_entry(declarations::IrEnumEntry& declaration, std::nullptr_t) {
  return visit_enum_entry(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:89-90
IrStatement& IrElementTransformerVoid::visit_field(declarations::IrField& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:92-93
IrStatement& IrElementTransformerVoid::visit_field(declarations::IrField& declaration, std::nullptr_t) {
  return visit_field(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:95-96
IrStatement& IrElementTransformerVoid::visit_local_delegated_property(declarations::IrLocalDelegatedProperty& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:98-99
IrStatement& IrElementTransformerVoid::visit_local_delegated_property(declarations::IrLocalDelegatedProperty& declaration, std::nullptr_t) {
  return visit_local_delegated_property(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:101-104
declarations::IrModuleFragment& IrElementTransformerVoid::visit_module_fragment(declarations::IrModuleFragment& declaration) {
  declaration.transform_children(*this, nullptr);
  return declaration;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:106-107
declarations::IrModuleFragment& IrElementTransformerVoid::visit_module_fragment(declarations::IrModuleFragment& declaration, std::nullptr_t) {
  return visit_module_fragment(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:109-110
IrStatement& IrElementTransformerVoid::visit_property(declarations::IrProperty& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:112-113
IrStatement& IrElementTransformerVoid::visit_property(declarations::IrProperty& declaration, std::nullptr_t) {
  return visit_property(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:115-116
IrStatement& IrElementTransformerVoid::visit_script(declarations::IrScript& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:118-119
IrStatement& IrElementTransformerVoid::visit_script(declarations::IrScript& declaration, std::nullptr_t) {
  return visit_script(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:121-122
IrStatement& IrElementTransformerVoid::visit_repl_snippet(declarations::IrReplSnippet& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:124-125
IrStatement& IrElementTransformerVoid::visit_repl_snippet(declarations::IrReplSnippet& declaration, std::nullptr_t) {
  return visit_repl_snippet(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:127-128
IrStatement& IrElementTransformerVoid::visit_simple_function(declarations::IrSimpleFunction& declaration) {
  return visit_function(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:130-131
IrStatement& IrElementTransformerVoid::visit_simple_function(declarations::IrSimpleFunction& declaration, std::nullptr_t) {
  return visit_simple_function(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:133-134
IrStatement& IrElementTransformerVoid::visit_type_alias(declarations::IrTypeAlias& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:136-137
IrStatement& IrElementTransformerVoid::visit_type_alias(declarations::IrTypeAlias& declaration, std::nullptr_t) {
  return visit_type_alias(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:139-140
IrStatement& IrElementTransformerVoid::visit_variable(declarations::IrVariable& declaration) {
  return visit_declaration(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:142-143
IrStatement& IrElementTransformerVoid::visit_variable(declarations::IrVariable& declaration, std::nullptr_t) {
  return visit_variable(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:145-148
declarations::IrPackageFragment& IrElementTransformerVoid::visit_package_fragment(declarations::IrPackageFragment& declaration) {
  declaration.transform_children(*this, nullptr);
  return declaration;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:150-151
IrElement& IrElementTransformerVoid::visit_package_fragment(declarations::IrPackageFragment& declaration, std::nullptr_t) {
  return visit_package_fragment(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:153-154
declarations::IrExternalPackageFragment& IrElementTransformerVoid::visit_external_package_fragment(declarations::IrExternalPackageFragment& declaration) {
  return dynamic_cast<declarations::IrExternalPackageFragment&>(visit_package_fragment(declaration));
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:156-157
declarations::IrExternalPackageFragment& IrElementTransformerVoid::visit_external_package_fragment(declarations::IrExternalPackageFragment& declaration, std::nullptr_t) {
  return visit_external_package_fragment(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:159-160
declarations::IrFile& IrElementTransformerVoid::visit_file(declarations::IrFile& declaration) {
  return dynamic_cast<declarations::IrFile&>(visit_package_fragment(declaration));
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:162-163
declarations::IrFile& IrElementTransformerVoid::visit_file(declarations::IrFile& declaration, std::nullptr_t) {
  return visit_file(declaration);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:165-168
expressions::IrExpression& IrElementTransformerVoid::visit_expression(expressions::IrExpression& expression) {
  expression.transform_children(*this, nullptr);
  return expression;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:170-171
expressions::IrExpression& IrElementTransformerVoid::visit_expression(expressions::IrExpression& expression, std::nullptr_t) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:173-176
expressions::IrBody& IrElementTransformerVoid::visit_body(expressions::IrBody& body) {
  body.transform_children(*this, nullptr);
  return body;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:178-179
expressions::IrBody& IrElementTransformerVoid::visit_body(expressions::IrBody& body, std::nullptr_t) {
  return visit_body(body);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:181-182
expressions::IrBody& IrElementTransformerVoid::visit_expression_body(expressions::IrExpressionBody& body) {
  return visit_body(body);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:184-185
expressions::IrBody& IrElementTransformerVoid::visit_expression_body(expressions::IrExpressionBody& body, std::nullptr_t) {
  return visit_expression_body(body);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:187-188
expressions::IrBody& IrElementTransformerVoid::visit_block_body(expressions::IrBlockBody& body) {
  return visit_body(body);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:190-191
expressions::IrBody& IrElementTransformerVoid::visit_block_body(expressions::IrBlockBody& body, std::nullptr_t) {
  return visit_block_body(body);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:193-194
expressions::IrExpression& IrElementTransformerVoid::visit_declaration_reference(expressions::IrDeclarationReference& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:196-197
expressions::IrExpression& IrElementTransformerVoid::visit_declaration_reference(expressions::IrDeclarationReference& expression, std::nullptr_t) {
  return visit_declaration_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:199-200
expressions::IrExpression& IrElementTransformerVoid::visit_member_access(expressions::IrMemberAccessExpression<symbols::IrSymbol>& expression) {
  return visit_declaration_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:202-203
IrElement& IrElementTransformerVoid::visit_member_access(expressions::IrMemberAccessExpression<symbols::IrSymbol>& expression, std::nullptr_t) {
  return visit_member_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:205-206
expressions::IrExpression& IrElementTransformerVoid::visit_function_access(expressions::IrFunctionAccessExpression& expression) {
  return visit_member_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:208-209
IrElement& IrElementTransformerVoid::visit_function_access(expressions::IrFunctionAccessExpression& expression, std::nullptr_t) {
  return visit_function_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:211-212
expressions::IrExpression& IrElementTransformerVoid::visit_constructor_call(expressions::IrConstructorCall& expression) {
  return visit_function_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:214-215
IrElement& IrElementTransformerVoid::visit_constructor_call(expressions::IrConstructorCall& expression, std::nullptr_t) {
  return visit_constructor_call(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:217-218
expressions::IrExpression& IrElementTransformerVoid::visit_annotation(expressions::IrAnnotation& expression) {
  return visit_constructor_call(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:220-221
IrElement& IrElementTransformerVoid::visit_annotation(expressions::IrAnnotation& expression, std::nullptr_t) {
  return visit_annotation(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:223-224
expressions::IrExpression& IrElementTransformerVoid::visit_singleton_reference(expressions::IrGetSingletonValue& expression) {
  return visit_declaration_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:226-227
expressions::IrExpression& IrElementTransformerVoid::visit_singleton_reference(expressions::IrGetSingletonValue& expression, std::nullptr_t) {
  return visit_singleton_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:229-230
expressions::IrExpression& IrElementTransformerVoid::visit_get_object_value(expressions::IrGetObjectValue& expression) {
  return visit_singleton_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:232-233
expressions::IrExpression& IrElementTransformerVoid::visit_get_object_value(expressions::IrGetObjectValue& expression, std::nullptr_t) {
  return visit_get_object_value(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:235-236
expressions::IrExpression& IrElementTransformerVoid::visit_get_enum_value(expressions::IrGetEnumValue& expression) {
  return visit_singleton_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:238-239
expressions::IrExpression& IrElementTransformerVoid::visit_get_enum_value(expressions::IrGetEnumValue& expression, std::nullptr_t) {
  return visit_get_enum_value(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:241-242
expressions::IrExpression& IrElementTransformerVoid::visit_raw_function_reference(expressions::IrRawFunctionReference& expression) {
  return visit_declaration_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:244-245
expressions::IrExpression& IrElementTransformerVoid::visit_raw_function_reference(expressions::IrRawFunctionReference& expression, std::nullptr_t) {
  return visit_raw_function_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:247-248
expressions::IrExpression& IrElementTransformerVoid::visit_container_expression(expressions::IrContainerExpression& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:250-251
expressions::IrExpression& IrElementTransformerVoid::visit_container_expression(expressions::IrContainerExpression& expression, std::nullptr_t) {
  return visit_container_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:253-254
expressions::IrExpression& IrElementTransformerVoid::visit_block(expressions::IrBlock& expression) {
  return visit_container_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:256-257
expressions::IrExpression& IrElementTransformerVoid::visit_block(expressions::IrBlock& expression, std::nullptr_t) {
  return visit_block(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:259-260
expressions::IrExpression& IrElementTransformerVoid::visit_composite(expressions::IrComposite& expression) {
  return visit_container_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:262-263
expressions::IrExpression& IrElementTransformerVoid::visit_composite(expressions::IrComposite& expression, std::nullptr_t) {
  return visit_composite(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:265-266
expressions::IrExpression& IrElementTransformerVoid::visit_returnable_block(expressions::IrReturnableBlock& expression) {
  return visit_block(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:268-269
expressions::IrExpression& IrElementTransformerVoid::visit_returnable_block(expressions::IrReturnableBlock& expression, std::nullptr_t) {
  return visit_returnable_block(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:271-272
expressions::IrExpression& IrElementTransformerVoid::visit_inlined_function_block(expressions::IrInlinedFunctionBlock& inlined_block) {
  return visit_block(inlined_block);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:274-275
expressions::IrExpression& IrElementTransformerVoid::visit_inlined_function_block(expressions::IrInlinedFunctionBlock& inlined_block, std::nullptr_t) {
  return visit_inlined_function_block(inlined_block);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:277-278
expressions::IrBody& IrElementTransformerVoid::visit_synthetic_body(expressions::IrSyntheticBody& body) {
  return visit_body(body);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:280-281
expressions::IrBody& IrElementTransformerVoid::visit_synthetic_body(expressions::IrSyntheticBody& body, std::nullptr_t) {
  return visit_synthetic_body(body);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:283-284
expressions::IrExpression& IrElementTransformerVoid::visit_break_continue(expressions::IrBreakContinue& jump) {
  return visit_expression(jump);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:286-287
expressions::IrExpression& IrElementTransformerVoid::visit_break_continue(expressions::IrBreakContinue& jump, std::nullptr_t) {
  return visit_break_continue(jump);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:289-290
expressions::IrExpression& IrElementTransformerVoid::visit_break(expressions::IrBreak& jump) {
  return visit_break_continue(jump);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:292-293
expressions::IrExpression& IrElementTransformerVoid::visit_break(expressions::IrBreak& jump, std::nullptr_t) {
  return visit_break(jump);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:295-296
expressions::IrExpression& IrElementTransformerVoid::visit_continue(expressions::IrContinue& jump) {
  return visit_break_continue(jump);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:298-299
expressions::IrExpression& IrElementTransformerVoid::visit_continue(expressions::IrContinue& jump, std::nullptr_t) {
  return visit_continue(jump);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:301-302
expressions::IrExpression& IrElementTransformerVoid::visit_call(expressions::IrCall& expression) {
  return visit_function_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:304-305
IrElement& IrElementTransformerVoid::visit_call(expressions::IrCall& expression, std::nullptr_t) {
  return visit_call(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:307-308
expressions::IrExpression& IrElementTransformerVoid::visit_callable_reference(expressions::IrCallableReference<symbols::IrSymbol>& expression) {
  return visit_member_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:310-311
IrElement& IrElementTransformerVoid::visit_callable_reference(expressions::IrCallableReference<symbols::IrSymbol>& expression, std::nullptr_t) {
  return visit_callable_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:313-314
expressions::IrExpression& IrElementTransformerVoid::visit_function_reference(expressions::IrFunctionReference& expression) {
  return visit_callable_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:316-317
IrElement& IrElementTransformerVoid::visit_function_reference(expressions::IrFunctionReference& expression, std::nullptr_t) {
  return visit_function_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:319-320
expressions::IrExpression& IrElementTransformerVoid::visit_property_reference(expressions::IrPropertyReference& expression) {
  return visit_callable_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:322-323
IrElement& IrElementTransformerVoid::visit_property_reference(expressions::IrPropertyReference& expression, std::nullptr_t) {
  return visit_property_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:325-326
expressions::IrExpression& IrElementTransformerVoid::visit_local_delegated_property_reference(expressions::IrLocalDelegatedPropertyReference& expression) {
  return visit_callable_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:328-329
IrElement& IrElementTransformerVoid::visit_local_delegated_property_reference(expressions::IrLocalDelegatedPropertyReference& expression, std::nullptr_t) {
  return visit_local_delegated_property_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:331-332
expressions::IrExpression& IrElementTransformerVoid::visit_rich_callable_reference(expressions::IrRichCallableReference<symbols::IrSymbol>& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:334-335
expressions::IrExpression& IrElementTransformerVoid::visit_rich_callable_reference(expressions::IrRichCallableReference<symbols::IrSymbol>& expression, std::nullptr_t) {
  return visit_rich_callable_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:337-338
expressions::IrExpression& IrElementTransformerVoid::visit_rich_function_reference(expressions::IrRichFunctionReference& expression) {
  return visit_rich_callable_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:340-341
expressions::IrExpression& IrElementTransformerVoid::visit_rich_function_reference(expressions::IrRichFunctionReference& expression, std::nullptr_t) {
  return visit_rich_function_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:343-344
expressions::IrExpression& IrElementTransformerVoid::visit_rich_property_reference(expressions::IrRichPropertyReference& expression) {
  return visit_rich_callable_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:346-347
expressions::IrExpression& IrElementTransformerVoid::visit_rich_property_reference(expressions::IrRichPropertyReference& expression, std::nullptr_t) {
  return visit_rich_property_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:349-350
expressions::IrExpression& IrElementTransformerVoid::visit_class_reference(expressions::IrClassReference& expression) {
  return visit_declaration_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:352-353
expressions::IrExpression& IrElementTransformerVoid::visit_class_reference(expressions::IrClassReference& expression, std::nullptr_t) {
  return visit_class_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:355-356
expressions::IrExpression& IrElementTransformerVoid::visit_const(expressions::IrConst& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:358-359
expressions::IrExpression& IrElementTransformerVoid::visit_const(expressions::IrConst& expression, std::nullptr_t) {
  return visit_const(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:361-364
expressions::IrConstantValue& IrElementTransformerVoid::visit_constant_value(expressions::IrConstantValue& expression) {
  expression.transform_children(*this, nullptr);
  return expression;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:366-367
expressions::IrConstantValue& IrElementTransformerVoid::visit_constant_value(expressions::IrConstantValue& expression, std::nullptr_t) {
  return visit_constant_value(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:369-370
expressions::IrConstantValue& IrElementTransformerVoid::visit_constant_primitive(expressions::IrConstantPrimitive& expression) {
  return visit_constant_value(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:372-373
expressions::IrConstantValue& IrElementTransformerVoid::visit_constant_primitive(expressions::IrConstantPrimitive& expression, std::nullptr_t) {
  return visit_constant_primitive(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:375-376
expressions::IrConstantValue& IrElementTransformerVoid::visit_constant_object(expressions::IrConstantObject& expression) {
  return visit_constant_value(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:378-379
expressions::IrConstantValue& IrElementTransformerVoid::visit_constant_object(expressions::IrConstantObject& expression, std::nullptr_t) {
  return visit_constant_object(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:381-382
expressions::IrConstantValue& IrElementTransformerVoid::visit_constant_array(expressions::IrConstantArray& expression) {
  return visit_constant_value(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:384-385
expressions::IrConstantValue& IrElementTransformerVoid::visit_constant_array(expressions::IrConstantArray& expression, std::nullptr_t) {
  return visit_constant_array(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:387-388
expressions::IrExpression& IrElementTransformerVoid::visit_delegating_constructor_call(expressions::IrDelegatingConstructorCall& expression) {
  return visit_function_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:390-391
IrElement& IrElementTransformerVoid::visit_delegating_constructor_call(expressions::IrDelegatingConstructorCall& expression, std::nullptr_t) {
  return visit_delegating_constructor_call(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:393-394
expressions::IrExpression& IrElementTransformerVoid::visit_dynamic_expression(expressions::IrDynamicExpression& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:396-397
expressions::IrExpression& IrElementTransformerVoid::visit_dynamic_expression(expressions::IrDynamicExpression& expression, std::nullptr_t) {
  return visit_dynamic_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:399-400
expressions::IrExpression& IrElementTransformerVoid::visit_dynamic_operator_expression(expressions::IrDynamicOperatorExpression& expression) {
  return visit_dynamic_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:402-403
expressions::IrExpression& IrElementTransformerVoid::visit_dynamic_operator_expression(expressions::IrDynamicOperatorExpression& expression, std::nullptr_t) {
  return visit_dynamic_operator_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:405-406
expressions::IrExpression& IrElementTransformerVoid::visit_dynamic_member_expression(expressions::IrDynamicMemberExpression& expression) {
  return visit_dynamic_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:408-409
expressions::IrExpression& IrElementTransformerVoid::visit_dynamic_member_expression(expressions::IrDynamicMemberExpression& expression, std::nullptr_t) {
  return visit_dynamic_member_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:411-412
expressions::IrExpression& IrElementTransformerVoid::visit_enum_constructor_call(expressions::IrEnumConstructorCall& expression) {
  return visit_function_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:414-415
IrElement& IrElementTransformerVoid::visit_enum_constructor_call(expressions::IrEnumConstructorCall& expression, std::nullptr_t) {
  return visit_enum_constructor_call(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:417-418
expressions::IrExpression& IrElementTransformerVoid::visit_error_expression(expressions::IrErrorExpression& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:420-421
expressions::IrExpression& IrElementTransformerVoid::visit_error_expression(expressions::IrErrorExpression& expression, std::nullptr_t) {
  return visit_error_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:423-424
expressions::IrExpression& IrElementTransformerVoid::visit_error_call_expression(expressions::IrErrorCallExpression& expression) {
  return visit_error_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:426-427
expressions::IrExpression& IrElementTransformerVoid::visit_error_call_expression(expressions::IrErrorCallExpression& expression, std::nullptr_t) {
  return visit_error_call_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:429-430
expressions::IrExpression& IrElementTransformerVoid::visit_field_access(expressions::IrFieldAccessExpression& expression) {
  return visit_declaration_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:432-433
expressions::IrExpression& IrElementTransformerVoid::visit_field_access(expressions::IrFieldAccessExpression& expression, std::nullptr_t) {
  return visit_field_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:435-436
expressions::IrExpression& IrElementTransformerVoid::visit_get_field(expressions::IrGetField& expression) {
  return visit_field_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:438-439
expressions::IrExpression& IrElementTransformerVoid::visit_get_field(expressions::IrGetField& expression, std::nullptr_t) {
  return visit_get_field(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:441-442
expressions::IrExpression& IrElementTransformerVoid::visit_set_field(expressions::IrSetField& expression) {
  return visit_field_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:444-445
expressions::IrExpression& IrElementTransformerVoid::visit_set_field(expressions::IrSetField& expression, std::nullptr_t) {
  return visit_set_field(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:447-448
expressions::IrExpression& IrElementTransformerVoid::visit_function_expression(expressions::IrFunctionExpression& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:450-451
IrElement& IrElementTransformerVoid::visit_function_expression(expressions::IrFunctionExpression& expression, std::nullptr_t) {
  return visit_function_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:453-454
expressions::IrExpression& IrElementTransformerVoid::visit_get_class(expressions::IrGetClass& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:456-457
expressions::IrExpression& IrElementTransformerVoid::visit_get_class(expressions::IrGetClass& expression, std::nullptr_t) {
  return visit_get_class(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:459-460
expressions::IrExpression& IrElementTransformerVoid::visit_instance_initializer_call(expressions::IrInstanceInitializerCall& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:462-463
expressions::IrExpression& IrElementTransformerVoid::visit_instance_initializer_call(expressions::IrInstanceInitializerCall& expression, std::nullptr_t) {
  return visit_instance_initializer_call(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:465-466
expressions::IrExpression& IrElementTransformerVoid::visit_loop(expressions::IrLoop& loop) {
  return visit_expression(loop);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:468-469
expressions::IrExpression& IrElementTransformerVoid::visit_loop(expressions::IrLoop& loop, std::nullptr_t) {
  return visit_loop(loop);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:471-472
expressions::IrExpression& IrElementTransformerVoid::visit_while_loop(expressions::IrWhileLoop& loop) {
  return visit_loop(loop);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:474-475
expressions::IrExpression& IrElementTransformerVoid::visit_while_loop(expressions::IrWhileLoop& loop, std::nullptr_t) {
  return visit_while_loop(loop);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:477-478
expressions::IrExpression& IrElementTransformerVoid::visit_do_while_loop(expressions::IrDoWhileLoop& loop) {
  return visit_loop(loop);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:480-481
expressions::IrExpression& IrElementTransformerVoid::visit_do_while_loop(expressions::IrDoWhileLoop& loop, std::nullptr_t) {
  return visit_do_while_loop(loop);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:483-484
expressions::IrExpression& IrElementTransformerVoid::visit_return(expressions::IrReturn& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:486-487
expressions::IrExpression& IrElementTransformerVoid::visit_return(expressions::IrReturn& expression, std::nullptr_t) {
  return visit_return(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:489-490
expressions::IrExpression& IrElementTransformerVoid::visit_string_concatenation(expressions::IrStringConcatenation& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:492-493
expressions::IrExpression& IrElementTransformerVoid::visit_string_concatenation(expressions::IrStringConcatenation& expression, std::nullptr_t) {
  return visit_string_concatenation(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:495-496
expressions::IrExpression& IrElementTransformerVoid::visit_suspension_point(expressions::IrSuspensionPoint& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:498-499
expressions::IrExpression& IrElementTransformerVoid::visit_suspension_point(expressions::IrSuspensionPoint& expression, std::nullptr_t) {
  return visit_suspension_point(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:501-502
expressions::IrExpression& IrElementTransformerVoid::visit_suspendable_expression(expressions::IrSuspendableExpression& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:504-505
expressions::IrExpression& IrElementTransformerVoid::visit_suspendable_expression(expressions::IrSuspendableExpression& expression, std::nullptr_t) {
  return visit_suspendable_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:507-508
expressions::IrExpression& IrElementTransformerVoid::visit_throw(expressions::IrThrow& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:510-511
expressions::IrExpression& IrElementTransformerVoid::visit_throw(expressions::IrThrow& expression, std::nullptr_t) {
  return visit_throw(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:513-514
expressions::IrExpression& IrElementTransformerVoid::visit_try(expressions::IrTry& a_try) {
  return visit_expression(a_try);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:516-517
expressions::IrExpression& IrElementTransformerVoid::visit_try(expressions::IrTry& a_try, std::nullptr_t) {
  return visit_try(a_try);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:519-522
expressions::IrCatch& IrElementTransformerVoid::visit_catch(expressions::IrCatch& a_catch) {
  a_catch.transform_children(*this, nullptr);
  return a_catch;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:524-525
expressions::IrCatch& IrElementTransformerVoid::visit_catch(expressions::IrCatch& a_catch, std::nullptr_t) {
  return visit_catch(a_catch);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:527-528
expressions::IrExpression& IrElementTransformerVoid::visit_type_operator(expressions::IrTypeOperatorCall& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:530-531
expressions::IrExpression& IrElementTransformerVoid::visit_type_operator(expressions::IrTypeOperatorCall& expression, std::nullptr_t) {
  return visit_type_operator(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:533-534
expressions::IrExpression& IrElementTransformerVoid::visit_value_access(expressions::IrValueAccessExpression& expression) {
  return visit_declaration_reference(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:536-537
expressions::IrExpression& IrElementTransformerVoid::visit_value_access(expressions::IrValueAccessExpression& expression, std::nullptr_t) {
  return visit_value_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:539-540
expressions::IrExpression& IrElementTransformerVoid::visit_get_value(expressions::IrGetValue& expression) {
  return visit_value_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:542-543
expressions::IrExpression& IrElementTransformerVoid::visit_get_value(expressions::IrGetValue& expression, std::nullptr_t) {
  return visit_get_value(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:545-546
expressions::IrExpression& IrElementTransformerVoid::visit_set_value(expressions::IrSetValue& expression) {
  return visit_value_access(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:548-549
expressions::IrExpression& IrElementTransformerVoid::visit_set_value(expressions::IrSetValue& expression, std::nullptr_t) {
  return visit_set_value(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:551-552
expressions::IrExpression& IrElementTransformerVoid::visit_vararg(expressions::IrVararg& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:554-555
expressions::IrExpression& IrElementTransformerVoid::visit_vararg(expressions::IrVararg& expression, std::nullptr_t) {
  return visit_vararg(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:557-560
expressions::IrSpreadElement& IrElementTransformerVoid::visit_spread_element(expressions::IrSpreadElement& spread) {
  spread.transform_children(*this, nullptr);
  return spread;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:562-563
expressions::IrSpreadElement& IrElementTransformerVoid::visit_spread_element(expressions::IrSpreadElement& spread, std::nullptr_t) {
  return visit_spread_element(spread);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:565-566
expressions::IrExpression& IrElementTransformerVoid::visit_when(expressions::IrWhen& expression) {
  return visit_expression(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:568-569
expressions::IrExpression& IrElementTransformerVoid::visit_when(expressions::IrWhen& expression, std::nullptr_t) {
  return visit_when(expression);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:571-574
expressions::IrBranch& IrElementTransformerVoid::visit_branch(expressions::IrBranch& branch) {
  branch.transform_children(*this, nullptr);
  return branch;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:576-577
expressions::IrBranch& IrElementTransformerVoid::visit_branch(expressions::IrBranch& branch, std::nullptr_t) {
  return visit_branch(branch);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:579-582
expressions::IrElseBranch& IrElementTransformerVoid::visit_else_branch(expressions::IrElseBranch& branch) {
  branch.transform_children(*this, nullptr);
  return branch;
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:584-585
expressions::IrElseBranch& IrElementTransformerVoid::visit_else_branch(expressions::IrElseBranch& branch, std::nullptr_t) {
  return visit_else_branch(branch);
}
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrElementTransformerVoid.kt:588-590
void transform_children_void(IrElement& element,
                             IrElementTransformerVoid& transformer) {
  element.transform_children(transformer, nullptr);
}
}  // namespace org::jetbrains::kotlin::ir::visitors
