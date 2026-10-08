/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:18-285
#pragma once

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
/** Auto-generated upstream by [org.jetbrains.kotlin.ir.generator.print.VisitorPrinter]. */
// NOTE(port): Kotlin out-R/in-D variance and generic star-projection views need
// their actual C++ type/view closure. Upper-bound specializations here do not
// establish conversions between different concrete symbol specializations.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:18-285
template <typename R, typename D>
class IrVisitor {
 public:
  virtual ~IrVisitor() = default;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:20-20
  virtual R visit_element(::org::jetbrains::kotlin::ir::IrElement& element, D data) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:22-23
  virtual R visit_declaration(declarations::IrDeclarationBase& declaration, D data) {
    return visit_element(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:25-26
  virtual R visit_value_parameter(declarations::IrValueParameter& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:28-29
  virtual R visit_class(declarations::IrClass& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:31-32
  virtual R visit_anonymous_initializer(declarations::IrAnonymousInitializer& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:34-35
  virtual R visit_type_parameter(declarations::IrTypeParameter& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:37-38
  virtual R visit_function(declarations::IrFunction& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:40-41
  virtual R visit_constructor(declarations::IrConstructor& declaration, D data) {
    return visit_function(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:43-44
  virtual R visit_enum_entry(declarations::IrEnumEntry& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:46-47
  virtual R visit_field(declarations::IrField& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:49-50
  virtual R visit_local_delegated_property(declarations::IrLocalDelegatedProperty& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:52-53
  virtual R visit_module_fragment(declarations::IrModuleFragment& declaration, D data) {
    return visit_element(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:55-56
  virtual R visit_property(declarations::IrProperty& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:58-59
  virtual R visit_script(declarations::IrScript& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:61-62
  virtual R visit_repl_snippet(declarations::IrReplSnippet& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:64-65
  virtual R visit_simple_function(declarations::IrSimpleFunction& declaration, D data) {
    return visit_function(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:67-68
  virtual R visit_type_alias(declarations::IrTypeAlias& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:70-71
  virtual R visit_variable(declarations::IrVariable& declaration, D data) {
    return visit_declaration(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:73-74
  virtual R visit_package_fragment(declarations::IrPackageFragment& declaration, D data) {
    return visit_element(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:76-77
  virtual R visit_external_package_fragment(declarations::IrExternalPackageFragment& declaration, D data) {
    return visit_package_fragment(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:79-80
  virtual R visit_file(declarations::IrFile& declaration, D data) {
    return visit_package_fragment(declaration, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:82-83
  virtual R visit_expression(expressions::IrExpression& expression, D data) {
    return visit_element(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:85-86
  virtual R visit_body(expressions::IrBody& body, D data) {
    return visit_element(body, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:88-89
  virtual R visit_expression_body(expressions::IrExpressionBody& body, D data) {
    return visit_body(body, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:91-92
  virtual R visit_block_body(expressions::IrBlockBody& body, D data) {
    return visit_body(body, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:94-95
  virtual R visit_declaration_reference(expressions::IrDeclarationReference& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:97-98
  virtual R visit_member_access(expressions::IrMemberAccessExpression<symbols::IrSymbol>& expression, D data) {
    return visit_declaration_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:100-101
  virtual R visit_function_access(expressions::IrFunctionAccessExpression& expression, D data) {
    return visit_member_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:103-104
  virtual R visit_constructor_call(expressions::IrConstructorCall& expression, D data) {
    return visit_function_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:106-107
  virtual R visit_annotation(expressions::IrAnnotation& expression, D data) {
    return visit_constructor_call(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:109-110
  virtual R visit_singleton_reference(expressions::IrGetSingletonValue& expression, D data) {
    return visit_declaration_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:112-113
  virtual R visit_get_object_value(expressions::IrGetObjectValue& expression, D data) {
    return visit_singleton_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:115-116
  virtual R visit_get_enum_value(expressions::IrGetEnumValue& expression, D data) {
    return visit_singleton_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:118-119
  virtual R visit_raw_function_reference(expressions::IrRawFunctionReference& expression, D data) {
    return visit_declaration_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:121-122
  virtual R visit_container_expression(expressions::IrContainerExpression& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:124-125
  virtual R visit_block(expressions::IrBlock& expression, D data) {
    return visit_container_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:127-128
  virtual R visit_composite(expressions::IrComposite& expression, D data) {
    return visit_container_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:130-131
  virtual R visit_returnable_block(expressions::IrReturnableBlock& expression, D data) {
    return visit_block(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:133-134
  virtual R visit_inlined_function_block(expressions::IrInlinedFunctionBlock& inlined_block, D data) {
    return visit_block(inlined_block, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:136-137
  virtual R visit_synthetic_body(expressions::IrSyntheticBody& body, D data) {
    return visit_body(body, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:139-140
  virtual R visit_break_continue(expressions::IrBreakContinue& jump, D data) {
    return visit_expression(jump, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:142-143
  virtual R visit_break(expressions::IrBreak& jump, D data) {
    return visit_break_continue(jump, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:145-146
  virtual R visit_continue(expressions::IrContinue& jump, D data) {
    return visit_break_continue(jump, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:148-149
  virtual R visit_call(expressions::IrCall& expression, D data) {
    return visit_function_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:151-152
  virtual R visit_callable_reference(expressions::IrCallableReference<symbols::IrSymbol>& expression, D data) {
    return visit_member_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:154-155
  virtual R visit_function_reference(expressions::IrFunctionReference& expression, D data) {
    return visit_callable_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:157-158
  virtual R visit_property_reference(expressions::IrPropertyReference& expression, D data) {
    return visit_callable_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:160-161
  virtual R visit_local_delegated_property_reference(expressions::IrLocalDelegatedPropertyReference& expression, D data) {
    return visit_callable_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:163-164
  virtual R visit_rich_callable_reference(expressions::IrRichCallableReference<symbols::IrSymbol>& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:166-167
  virtual R visit_rich_function_reference(expressions::IrRichFunctionReference& expression, D data) {
    return visit_rich_callable_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:169-170
  virtual R visit_rich_property_reference(expressions::IrRichPropertyReference& expression, D data) {
    return visit_rich_callable_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:172-173
  virtual R visit_class_reference(expressions::IrClassReference& expression, D data) {
    return visit_declaration_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:175-176
  virtual R visit_const(expressions::IrConst& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:178-179
  virtual R visit_constant_value(expressions::IrConstantValue& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:181-182
  virtual R visit_constant_primitive(expressions::IrConstantPrimitive& expression, D data) {
    return visit_constant_value(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:184-185
  virtual R visit_constant_object(expressions::IrConstantObject& expression, D data) {
    return visit_constant_value(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:187-188
  virtual R visit_constant_array(expressions::IrConstantArray& expression, D data) {
    return visit_constant_value(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:190-191
  virtual R visit_delegating_constructor_call(expressions::IrDelegatingConstructorCall& expression, D data) {
    return visit_function_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:193-194
  virtual R visit_dynamic_expression(expressions::IrDynamicExpression& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:196-197
  virtual R visit_dynamic_operator_expression(expressions::IrDynamicOperatorExpression& expression, D data) {
    return visit_dynamic_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:199-200
  virtual R visit_dynamic_member_expression(expressions::IrDynamicMemberExpression& expression, D data) {
    return visit_dynamic_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:202-203
  virtual R visit_enum_constructor_call(expressions::IrEnumConstructorCall& expression, D data) {
    return visit_function_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:205-206
  virtual R visit_error_expression(expressions::IrErrorExpression& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:208-209
  virtual R visit_error_call_expression(expressions::IrErrorCallExpression& expression, D data) {
    return visit_error_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:211-212
  virtual R visit_field_access(expressions::IrFieldAccessExpression& expression, D data) {
    return visit_declaration_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:214-215
  virtual R visit_get_field(expressions::IrGetField& expression, D data) {
    return visit_field_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:217-218
  virtual R visit_set_field(expressions::IrSetField& expression, D data) {
    return visit_field_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:220-221
  virtual R visit_function_expression(expressions::IrFunctionExpression& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:223-224
  virtual R visit_get_class(expressions::IrGetClass& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:226-227
  virtual R visit_instance_initializer_call(expressions::IrInstanceInitializerCall& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:229-230
  virtual R visit_loop(expressions::IrLoop& loop, D data) {
    return visit_expression(loop, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:232-233
  virtual R visit_while_loop(expressions::IrWhileLoop& loop, D data) {
    return visit_loop(loop, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:235-236
  virtual R visit_do_while_loop(expressions::IrDoWhileLoop& loop, D data) {
    return visit_loop(loop, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:238-239
  virtual R visit_return(expressions::IrReturn& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:241-242
  virtual R visit_string_concatenation(expressions::IrStringConcatenation& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:244-245
  virtual R visit_suspension_point(expressions::IrSuspensionPoint& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:247-248
  virtual R visit_suspendable_expression(expressions::IrSuspendableExpression& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:250-251
  virtual R visit_throw(expressions::IrThrow& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:253-254
  virtual R visit_try(expressions::IrTry& a_try, D data) {
    return visit_expression(a_try, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:256-257
  virtual R visit_catch(expressions::IrCatch& a_catch, D data) {
    return visit_element(a_catch, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:259-260
  virtual R visit_type_operator(expressions::IrTypeOperatorCall& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:262-263
  virtual R visit_value_access(expressions::IrValueAccessExpression& expression, D data) {
    return visit_declaration_reference(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:265-266
  virtual R visit_get_value(expressions::IrGetValue& expression, D data) {
    return visit_value_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:268-269
  virtual R visit_set_value(expressions::IrSetValue& expression, D data) {
    return visit_value_access(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:271-272
  virtual R visit_vararg(expressions::IrVararg& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:274-275
  virtual R visit_spread_element(expressions::IrSpreadElement& spread, D data) {
    return visit_element(spread, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:277-278
  virtual R visit_when(expressions::IrWhen& expression, D data) {
    return visit_expression(expression, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:280-281
  virtual R visit_branch(expressions::IrBranch& branch, D data) {
    return visit_element(branch, data);
  }
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:283-284
  virtual R visit_else_branch(expressions::IrElseBranch& branch, D data) {
    return visit_branch(branch, data);
  }
};
}  // namespace org::jetbrains::kotlin::ir::visitors
