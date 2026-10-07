# Immediate Actions - High-Value Files

Based on AST analysis, here are the concrete next steps.

## Summary

- **Files Present:** 98/676 (14.5%)
- **Function parity:** 453/7653 matched (target 979) — 5.9%
- **Class/type parity:** 156/1725 matched (target 263) — 9.0%
- **Combined symbol parity:** 609/9378 matched (target 1242) — 6.5%
- **Average inline-code cosine:** 0.34 (function body across 98 matched files)
- **Average documentation cosine:** 0.61 (doc text across 98 matched files)
- **Cheat-zeroed Files:** 24
- **Critical Issues:** 69 files with <0.60 function similarity
- **Needs Review:** 2 files with 0.60-0.84 function similarity
- **Excellent:** 27 files with >=0.85 function similarity

## Priority 1: Fix Incomplete High-Dependency Files

### 1. ir.IrElement
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 79
- **Priority Score:** 79000104.0
- **Functions:** 0/0 matched (target 4)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

### 2. expressions.IrExpression
- **Similarity:** 0.42 (needs 43% improvement)
- **Dependencies:** 62
- **Priority Score:** 62000204.0
- **Functions:** 1/1 matched (target 2)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

### 3. types.IrType
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 59
- **Priority Score:** 59010912.0
- **Functions:** 0/1 matched (target 7)
- **Missing functions:** `SimpleTypeNullability::fromHasQuestionMark`
- **Types:** 8/8 matched (target 11)
- **Missing types:** _none_
- **Symbol Deficit:** 1 (functions: 1, types: 0)
- **Action:** Deep review - likely missing major functionality

### 4. visitors.IrElementTransformerVoid
- **Similarity:** 0.68 (needs 17% improvement)
- **Dependencies:** 42
- **Priority Score:** 42018204.0
- **Functions:** 181/181 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Action:** Review and complete missing sections

### 5. declarations.IrClass
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 31
- **Priority Score:** 31030410.0
- **Functions:** 0/3 matched (target 4)
- **Missing functions:** `IrClass::accept`, `IrClass::acceptChildren`, `IrClass::transformChildren`
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_
- **Symbol Deficit:** 3 (functions: 3, types: 0)
- **Action:** Deep review - likely missing major functionality

### 6. symbols.IrSymbol
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 28
- **Priority Score:** 28172310.0
- **Functions:** 0/0 matched (target 11)
- **Missing functions:** _none_
- **Types:** 6/23 matched (target 7)
- **Missing types:** `IrPackageFragmentSymbol`, `IrFileSymbol`, `IrExternalPackageFragmentSymbol`, `IrAnonymousInitializerSymbol`, `IrEnumEntrySymbol`, `IrFieldSymbol`, `IrScriptSymbol`, `IrReplSnippetSymbol`, `IrReturnTargetSymbol`, `IrFunctionSymbol`, `IrConstructorSymbol`, `IrSimpleFunctionSymbol`, `IrReturnableBlockSymbol`, `IrDeclarationWithAccessorsSymbol`, `IrPropertySymbol`, `IrLocalDelegatedPropertySymbol`, `IrTypeAliasSymbol`
- **Symbol Deficit:** 17 (functions: 0, types: 17)
- **Action:** Deep review - likely missing major functionality

### 7. descriptors.Modality
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 25
- **Priority Score:** 25010210.0
- **Functions:** 0/1 matched
- **Missing functions:** `Modality::convertFromFlags`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Symbol Deficit:** 1 (functions: 1, types: 0)
- **Action:** Deep review - likely missing major functionality

### 8. declarations.IrVariable
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 22
- **Priority Score:** 22030410.0
- **Functions:** 0/3 matched (target 4)
- **Missing functions:** `IrVariable::accept`, `IrVariable::acceptChildren`, `IrVariable::transformChildren`
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_
- **Symbol Deficit:** 3 (functions: 3, types: 0)
- **Action:** Deep review - likely missing major functionality

### 9. ir.IrAttribute
- **Similarity:** 0.13 (needs 72% improvement)
- **Dependencies:** 20
- **Priority Score:** 20071708.0
- **Functions:** 7/14 matched (target 21)
- **Missing functions:** `IrAttribute::Flag::getValue`, `IrAttribute::Flag::setValue`, `IrAttribute::Flag::get`, `IrAttribute::Flag::set`, `IrAttribute::Flag::Delegate::provideDelegate`, `IrAttribute::Delegate::create`, `IrAttribute::Delegate::provideDelegate`
- **Types:** 3/3 matched
- **Missing types:** _none_
- **Symbol Deficit:** 7 (functions: 7, types: 0)
- **Action:** Deep review - likely missing major functionality

### 10. expressions.IrBody
- **Similarity:** 0.42 (needs 43% improvement)
- **Dependencies:** 19
- **Priority Score:** 19000206.0
- **Functions:** 1/1 matched (target 2)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

### 11. expressions.IrGetValue
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 11
- **Priority Score:** 11010210.0
- **Functions:** 0/1 matched
- **Missing functions:** `IrGetValue::accept`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Symbol Deficit:** 1 (functions: 1, types: 0)
- **Action:** Deep review - likely missing major functionality

### 12. descriptors.ClassKind
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 11
- **Priority Score:** 11000110.0
- **Functions:** 0/0 matched (target 8)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

## Priority 2: Port Missing High-Value Files

Critical missing files (>10 dependencies):

1. **common.CommonBackendContext** (58 deps)
   - Path: `compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/CommonBackendContext.kt`
   - Essential for 58 other files

2. **phaser.PhasePrerequisites** (34 deps)
   - Path: `compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/phaser/PhasePrerequisites.kt`
   - Essential for 34 other files

3. **konan.NativeLoweringContext** (33 deps)
   - Path: `kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/NativeLoweringContext.kt`
   - Essential for 33 other files

4. **common.LoweringContext** (31 deps)
   - Path: `compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/LoweringContext.kt`
   - Essential for 31 other files

5. **name.FqName** (30 deps)
   - Path: `core/names/src/org/jetbrains/kotlin/name/FqName.kt`
   - Essential for 30 other files

6. **visitors.IrVisitor** (25 deps)
   - Path: `compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt`
   - Essential for 25 other files

7. **konan.NativeGenerationState** (24 deps)
   - Path: `kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/NativeGenerationState.kt`
   - Essential for 24 other files

8. **concurrent.Internal** (18 deps)
   - Path: `kotlin-native/runtime/src/main/kotlin/kotlin/native/concurrent/Internal.kt`
   - Essential for 18 other files

9. **konan.NativeBackendContext** (16 deps)
   - Path: `kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/NativeBackendContext.kt`
   - Essential for 16 other files

10. **common.Lower** (13 deps)
   - Path: `compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/Lower.kt`
   - Essential for 13 other files

11. **declarations.IrDeclarationOrigin** (13 deps)
   - Path: `compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrDeclarationOrigin.kt`
   - Essential for 13 other files

12. **internal.IntrinsicType** (10 deps)
   - Path: `kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/IntrinsicType.kt`
   - Essential for 10 other files

13. **common.IrElementTransformerVoidWithContext** (10 deps)
   - Path: `compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/IrElementTransformerVoidWithContext.kt`
   - Essential for 10 other files

14. **internal.NativePtr** (10 deps)
   - Path: `kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/NativePtr.kt`
   - Essential for 10 other files

## Detailed Work Items

Every matched file is listed below with function and type symbol parity.

### 1. ir.IrElement

- **Target:** `ir.IrElement [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 79
- **Priority Score:** 79000104.0
- **Functions:** 0/0 matched (target 4)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 2. ir.IrStatement

- **Target:** `ir.IrStatement`
- **Similarity:** 1.00
- **Dependents:** 65
- **Priority Score:** 65000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 3. expressions.IrExpression

- **Target:** `expressions.IrExpression`
- **Similarity:** 0.42
- **Dependents:** 62
- **Priority Score:** 62000204.0
- **Functions:** 1/1 matched (target 2)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 4. types.IrType

- **Target:** `types.IrType`
- **Similarity:** 0.00
- **Dependents:** 59
- **Priority Score:** 59010912.0
- **Functions:** 0/1 matched (target 7)
- **Missing functions:** `SimpleTypeNullability::fromHasQuestionMark`
- **Types:** 8/8 matched (target 11)
- **Missing types:** _none_
- **Lint issues:** 1

### 5. visitors.IrElementTransformerVoid

- **Target:** `visitors.IrElementTransformerVoid`
- **Similarity:** 0.68
- **Dependents:** 42
- **Priority Score:** 42018204.0
- **Functions:** 181/181 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 89

### 6. declarations.IrClass

- **Target:** `declarations.IrClass`
- **Similarity:** 0.00
- **Dependents:** 31
- **Priority Score:** 31030410.0
- **Functions:** 0/3 matched (target 4)
- **Missing functions:** `IrClass::accept`, `IrClass::acceptChildren`, `IrClass::transformChildren`
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_

### 7. declarations.IrDeclaration

- **Target:** `declarations.IrDeclaration`
- **Similarity:** 1.00
- **Dependents:** 30
- **Priority Score:** 30000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_

### 8. symbols.IrSymbol

- **Target:** `symbols.IrValueSymbol [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 28
- **Priority Score:** 28172310.0
- **Functions:** 0/0 matched (target 11)
- **Missing functions:** _none_
- **Types:** 6/23 matched (target 7)
- **Missing types:** `IrPackageFragmentSymbol`, `IrFileSymbol`, `IrExternalPackageFragmentSymbol`, `IrAnonymousInitializerSymbol`, `IrEnumEntrySymbol`, `IrFieldSymbol`, `IrScriptSymbol`, `IrReplSnippetSymbol`, `IrReturnTargetSymbol`, `IrFunctionSymbol`, `IrConstructorSymbol`, `IrSimpleFunctionSymbol`, `IrReturnableBlockSymbol`, `IrDeclarationWithAccessorsSymbol`, `IrPropertySymbol`, `IrLocalDelegatedPropertySymbol`, `IrTypeAliasSymbol`

### 9. visitors.IrTransformer

- **Target:** `visitors.IrTransformer`
- **Similarity:** 0.86
- **Dependents:** 27
- **Priority Score:** 27009002.0
- **Functions:** 89/89 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 10. descriptors.Modality

- **Target:** `descriptors.Modality`
- **Similarity:** 0.00
- **Dependents:** 25
- **Priority Score:** 25010210.0
- **Functions:** 0/1 matched
- **Missing functions:** `Modality::convertFromFlags`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 11. declarations.IrVariable

- **Target:** `declarations.IrVariable`
- **Similarity:** 0.00
- **Dependents:** 22
- **Priority Score:** 22030410.0
- **Functions:** 0/3 matched (target 4)
- **Missing functions:** `IrVariable::accept`, `IrVariable::acceptChildren`, `IrVariable::transformChildren`
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_

### 12. ir.IrAttribute

- **Target:** `ir.IrAttribute`
- **Similarity:** 0.13
- **Dependents:** 20
- **Priority Score:** 20071708.0
- **Functions:** 7/14 matched (target 21)
- **Missing functions:** `IrAttribute::Flag::getValue`, `IrAttribute::Flag::setValue`, `IrAttribute::Flag::get`, `IrAttribute::Flag::set`, `IrAttribute::Flag::Delegate::provideDelegate`, `IrAttribute::Delegate::create`, `IrAttribute::Delegate::provideDelegate`
- **Types:** 3/3 matched
- **Missing types:** _none_
- **Lint issues:** 6

### 13. expressions.IrBody

- **Target:** `expressions.IrBody`
- **Similarity:** 0.42
- **Dependents:** 19
- **Priority Score:** 19000206.0
- **Functions:** 1/1 matched (target 2)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 14. expressions.IrGetValue

- **Target:** `expressions.IrGetValue`
- **Similarity:** 0.00
- **Dependents:** 11
- **Priority Score:** 11010210.0
- **Functions:** 0/1 matched
- **Missing functions:** `IrGetValue::accept`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 15. descriptors.ClassKind

- **Target:** `descriptors.ClassKind [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 11
- **Priority Score:** 11000110.0
- **Functions:** 0/0 matched (target 8)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 16. reflect.KClass

- **Target:** `reflect.KClass`
- **Similarity:** 1.00
- **Dependents:** 10
- **Priority Score:** 10000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 17. types.Variance

- **Target:** `types.Variance`
- **Similarity:** 0.00
- **Dependents:** 9
- **Priority Score:** 9040510.0
- **Functions:** 0/4 matched (target 8)
- **Missing functions:** `Variance::allowsPosition`, `Variance::superpose`, `Variance::opposite`, `Variance::toString`
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 18. impl.IrGetValueImpl

- **Target:** `impl.IrGetValueImpl [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 8
- **Priority Score:** 8000110.0
- **Functions:** 0/0 matched (target 13)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 19. llvm.CodeGenerator

- **Target:** `kotlinc_native_ref.CodeGenerator`
- **Similarity:** 0.10
- **Dependents:** 6
- **Priority Score:** 7276009.0
- **Functions:** 32/145 matched (target 40)
- **Missing functions:** `CodeGenerator::addFunctionDefinition`, `CodeGenerator::getLlvmFunctionFrom`, `CodeGenerator::getLlvmFunctionOrNullFrom`, `CodeGenerator::typeInfoValue`, `CodeGenerator::param`, `CodeGenerator::functionEntryPointAddress`, `CodeGenerator::typeInfoForAllocation`, `CodeGenerator::generateLocationInfo`, `ExceptionHandler::genThrow`, `generateFunction`, `FunctionGenerationContextBuilder<T>::generate`, `generateFunction`, `generateFunctionNoRuntime`, `generateFunctionBody`, `VirtualTablesLookup::getInterfaceTableRecord`, `VirtualTablesLookup::fastPath`, `VirtualTablesLookup::checkIsSubtype`, `VirtualTablesLookup::getVirtualImpl`, `CodeGenerator::getVirtualFunctionTrampoline`, `CodeGenerator::getVirtualFunctionTrampolineImpl`, `StackLocalsManagerImpl::enterScope`, `StackLocalsManagerImpl::exitScope`, `StackLocalsManagerImpl::isRootScope`, `StackLocalsManagerImpl::isEmpty`, `StackLocalsManagerImpl::createRootSetSlot`, `StackLocalsManagerImpl::alloc`, `StackLocalsManagerImpl::localArrayType`, `StackLocalsManagerImpl::allocArray`, `StackLocalsManagerImpl::clean`, `StackLocalsManagerImpl::clean`, `StackLocalsManagerImpl::setTypeInfoForStackObject`, `FunctionGenerationContext::update`, `FunctionGenerationContext::dispose`, `FunctionGenerationContext::basicBlockInFunction`, `FunctionGenerationContext::moveBlockAfterEntry`, `FunctionGenerationContext::alloca`, `FunctionGenerationContext::param`, `FunctionGenerationContext::applyMemoryOrderAndAlignment`, `FunctionGenerationContext::load`, `FunctionGenerationContext::loadSlot`, `FunctionGenerationContext::store`, `FunctionGenerationContext::storeHeapRef`, `FunctionGenerationContext::storeStackRef`, `FunctionGenerationContext::storeAny`, `FunctionGenerationContext::updateReturnRef`, `FunctionGenerationContext::updateRef`, `FunctionGenerationContext::switchThreadState`, `FunctionGenerationContext::memset`, `FunctionGenerationContext::call`, `FunctionGenerationContext::callRaw`, `FunctionGenerationContext::allocInstance`, `FunctionGenerationContext::allocInstance`, `FunctionGenerationContext::allocArray`, `FunctionGenerationContext::unreachable`, `FunctionGenerationContext::not`, `FunctionGenerationContext::and`, `FunctionGenerationContext::or`, `FunctionGenerationContext::xor`, `FunctionGenerationContext::zext`, `FunctionGenerationContext::sext`, `FunctionGenerationContext::ext`, `FunctionGenerationContext::trunc`, `FunctionGenerationContext::shift`, `FunctionGenerationContext::shl`, `FunctionGenerationContext::shr`, `FunctionGenerationContext::fcmpEq`, `FunctionGenerationContext::fcmpGt`, `FunctionGenerationContext::fcmpGe`, `FunctionGenerationContext::fcmpLt`, `FunctionGenerationContext::fcmpLe`, `FunctionGenerationContext::sub`, `FunctionGenerationContext::add`, `FunctionGenerationContext::fsub`, `FunctionGenerationContext::fadd`, `FunctionGenerationContext::fneg`, `FunctionGenerationContext::select`, `FunctionGenerationContext::bitcast`, `FunctionGenerationContext::intToPtr`, `FunctionGenerationContext::ptrToInt`, `FunctionGenerationContext::gep`, `FunctionGenerationContext::structGep`, `FunctionGenerationContext::extractValue`, `FunctionGenerationContext::gxxLandingpad`, `FunctionGenerationContext::extractElement`, `FunctionGenerationContext::filteringExceptionHandler`, `FunctionGenerationContext::terminateWithCurrentException`, `FunctionGenerationContext::terminate`, `FunctionGenerationContext::kotlinExceptionHandler`, `FunctionGenerationContext::catchKotlinException`, `FunctionGenerationContext::extractKotlinException`, `FunctionGenerationContext::createForeignException`, `FunctionGenerationContext::generateFrameCheck`, `FunctionGenerationContext::debugLocation`, `FunctionGenerationContext::loadTypeInfo`, `FunctionGenerationContext::getEnumEntry`, `FunctionGenerationContext::getObjCClass`, `FunctionGenerationContext::getObjCClass`, `FunctionGenerationContext::getObjCClassFromNativeRuntime`, `FunctionGenerationContext::resetDebugLocation`, `FunctionGenerationContext::position`, `FunctionGenerationContext::mapParameterForDebug`, `FunctionGenerationContext::prologue`, `FunctionGenerationContext::epilogue`, `FunctionGenerationContext::retValue`, `FunctionGenerationContext::retVoid`, `FunctionGenerationContext::onReturn`, `FunctionGenerationContext::handleEpilogueExperimentalMM`, `FunctionGenerationContext::PositionHolder::resetBuilderDebugLocation`, `FunctionGenerationContext::PositionHolder::setBuilderDebugLocation`, `FunctionGenerationContext::PositionHolder::hasDebugLocation`, `FunctionGenerationContext::releaseVars`, `DefaultFunctionGenerationContext::ret`, `DefaultFunctionGenerationContext::processReturns`
- **Types:** 2/15 matched (target 2)
- **Missing types:** `CodeGenerator`, `ExceptionHandler`, `None`, `Caller`, `Local`, `ThreadState`, `VirtualTablesLookup`, `LocationInfoRange`, `StackLocalsManager`, `StackLocalsManagerImpl`, `StackLocal`, `FunctionGenerationContextBuilder`, `DefaultFunctionGenerationContext`
- **Lint issues:** 2

### 20. declarations.IrSymbolOwner

- **Target:** `declarations.IrSymbolOwner [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 7
- **Priority Score:** 7000110.0
- **Functions:** 0/0 matched (target 1)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 21. declarations.IrParameterKind

- **Target:** `declarations.IrParameterKind`
- **Similarity:** 1.00
- **Dependents:** 6
- **Priority Score:** 6000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 22. expressions.IrSuspensionPoint

- **Target:** `expressions.IrSuspensionPoint`
- **Similarity:** 0.00
- **Dependents:** 5
- **Priority Score:** 5030410.0
- **Functions:** 0/3 matched
- **Missing functions:** `IrSuspensionPoint::accept`, `IrSuspensionPoint::acceptChildren`, `IrSuspensionPoint::transformChildren`
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 23. expressions.IrSetValue

- **Target:** `expressions.IrSetValue`
- **Similarity:** 0.00
- **Dependents:** 5
- **Priority Score:** 5030410.0
- **Functions:** 0/3 matched
- **Missing functions:** `IrSetValue::accept`, `IrSetValue::acceptChildren`, `IrSetValue::transformChildren`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 24. declarations.IrDeclarationBase

- **Target:** `declarations.IrDeclarationBase [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 5
- **Priority Score:** 5000110.0
- **Functions:** 0/0 matched (target 3)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 25. declarations.IrDeclarationWithName

- **Target:** `declarations.IrDeclarationWithName`
- **Similarity:** 1.00
- **Dependents:** 5
- **Priority Score:** 5000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 26. ir.IrElementBase

- **Target:** `ir.IrElementBase`
- **Similarity:** 0.25
- **Dependents:** 4
- **Priority Score:** 4031207.5
- **Functions:** 8/11 matched (target 13)
- **Missing functions:** `IrElementBase::transform`, `IrElementBase::acceptChildren`, `IrElementBase::transformChildren`
- **Types:** 1/1 matched (target 4)
- **Missing types:** _none_
- **Lint issues:** 4

### 27. descriptors.DescriptorVisibility

- **Target:** `descriptors.DescriptorVisibility`
- **Similarity:** 0.27
- **Dependents:** 4
- **Priority Score:** 4030807.2
- **Functions:** 4/6 matched
- **Missing functions:** `DelegatedDescriptorVisibility::mustCheckInImports`, `DelegatedDescriptorVisibility::normalize`
- **Types:** 1/2 matched
- **Missing types:** `DelegatedDescriptorVisibility`
- **Lint issues:** 2

### 28. impl.IrVariableImpl

- **Target:** `impl.IrVariableImpl [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 4
- **Priority Score:** 4000110.0
- **Functions:** 0/0 matched (target 26)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 29. reflect.KProperty

- **Target:** `reflect.KProperty`
- **Similarity:** 1.00
- **Dependents:** 3
- **Priority Score:** 3070800.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/8 matched (target 1)
- **Missing types:** `KProperty0`, `KProperty1`, `KProperty2`, `KMutableProperty`, `KMutableProperty0`, `KMutableProperty1`, `KMutableProperty2`

### 30. expressions.IrExpressionBody

- **Target:** `expressions.IrExpressionBody`
- **Similarity:** 0.11
- **Dependents:** 3
- **Priority Score:** 3030509.0
- **Functions:** 1/4 matched (target 5)
- **Missing functions:** `IrExpressionBody::accept`, `IrExpressionBody::acceptChildren`, `IrExpressionBody::transformChildren`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 31. declarations.IrValueParameter

- **Target:** `declarations.IrValueParameter`
- **Similarity:** 0.11
- **Dependents:** 3
- **Priority Score:** 3030509.0
- **Functions:** 1/4 matched (target 8)
- **Missing functions:** `IrValueParameter::accept`, `IrValueParameter::acceptChildren`, `IrValueParameter::transformChildren`
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_
- **Lint issues:** 3

### 32. generated._ArraysNative

- **Target:** `collections.ArraysNative`
- **Similarity:** 0.01
- **Dependents:** 0
- **Priority Score:** 2334010.0
- **Functions:** 9/240 matched (target 11)
- **Missing functions:** `Array<out T>::elementAt`, `ByteArray::elementAt`, `ShortArray::elementAt`, `IntArray::elementAt`, `LongArray::elementAt`, `FloatArray::elementAt`, `DoubleArray::elementAt`, `BooleanArray::elementAt`, `CharArray::elementAt`, `Array<out T>::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `ByteArray::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `ShortArray::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `IntArray::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `LongArray::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `FloatArray::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `DoubleArray::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `BooleanArray::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `CharArray::asList`, `isEmpty`, `contains`, `get`, `indexOf`, `lastIndexOf`, `iterator`, `next`, `hasNext`, `Array<out T>::contentDeepEquals`, `contentDeepEquals`, `Array<out T>::contentDeepHashCode`, `contentDeepHashCode`, `Array<out T>::contentDeepToString`, `contentDeepToString`, `contentEquals`, `contentEquals`, `contentEquals`, `contentEquals`, `contentEquals`, `contentEquals`, `contentEquals`, `contentEquals`, `contentEquals`, `contentHashCode`, `contentHashCode`, `contentHashCode`, `contentHashCode`, `contentHashCode`, `contentHashCode`, `contentHashCode`, `contentHashCode`, `contentHashCode`, `contentToString`, `contentToString`, `contentToString`, `contentToString`, `contentToString`, `contentToString`, `contentToString`, `contentToString`, `contentToString`, `Array<out T>::copyInto`, `ByteArray::copyInto`, `ShortArray::copyInto`, `LongArray::copyInto`, `FloatArray::copyInto`, `DoubleArray::copyInto`, `BooleanArray::copyInto`, `CharArray::copyInto`, `Array<T>::copyOf`, `ByteArray::copyOf`, `ShortArray::copyOf`, `LongArray::copyOf`, `FloatArray::copyOf`, `DoubleArray::copyOf`, `BooleanArray::copyOf`, `CharArray::copyOf`, `ByteArray::copyOf`, `ShortArray::copyOf`, `LongArray::copyOf`, `FloatArray::copyOf`, `DoubleArray::copyOf`, `BooleanArray::copyOf`, `CharArray::copyOf`, `ByteArray::copyOfRange`, `ShortArray::copyOfRange`, `LongArray::copyOfRange`, `FloatArray::copyOfRange`, `DoubleArray::copyOfRange`, `BooleanArray::copyOfRange`, `CharArray::copyOfRange`, `Array<T>::copyOfUninitializedElements`, `ByteArray::copyOfUninitializedElements`, `ShortArray::copyOfUninitializedElements`, `LongArray::copyOfUninitializedElements`, `FloatArray::copyOfUninitializedElements`, `DoubleArray::copyOfUninitializedElements`, `BooleanArray::copyOfUninitializedElements`, `CharArray::copyOfUninitializedElements`, `Array<T>::copyOfUninitializedElements`, `ByteArray::copyOfUninitializedElements`, `ShortArray::copyOfUninitializedElements`, `LongArray::copyOfUninitializedElements`, `FloatArray::copyOfUninitializedElements`, `DoubleArray::copyOfUninitializedElements`, `BooleanArray::copyOfUninitializedElements`, `CharArray::copyOfUninitializedElements`, `Array<T>::fill`, `ByteArray::fill`, `ShortArray::fill`, `LongArray::fill`, `FloatArray::fill`, `DoubleArray::fill`, `BooleanArray::fill`, `CharArray::fill`, `Array<T>::plus`, `ByteArray::plus`, `ShortArray::plus`, `IntArray::plus`, `LongArray::plus`, `FloatArray::plus`, `DoubleArray::plus`, `BooleanArray::plus`, `CharArray::plus`, `Array<T>::plus`, `ByteArray::plus`, `ShortArray::plus`, `IntArray::plus`, `LongArray::plus`, `FloatArray::plus`, `DoubleArray::plus`, `BooleanArray::plus`, `CharArray::plus`, `Array<T>::plus`, `ByteArray::plus`, `ShortArray::plus`, `IntArray::plus`, `LongArray::plus`, `FloatArray::plus`, `DoubleArray::plus`, `BooleanArray::plus`, `CharArray::plus`, `Array<T>::plusElement`, `IntArray::sort`, `LongArray::sort`, `ByteArray::sort`, `ShortArray::sort`, `DoubleArray::sort`, `FloatArray::sort`, `CharArray::sort`, `Array<out T>::sort`, `Array<out T>::sort`, `ByteArray::sort`, `ShortArray::sort`, `IntArray::sort`, `LongArray::sort`, `FloatArray::sort`, `DoubleArray::sort`, `CharArray::sort`, `Array<out T>::sortWith`, `Array<out T>::sortWith`, `ByteArray::toTypedArray`, `ShortArray::toTypedArray`, `IntArray::toTypedArray`, `LongArray::toTypedArray`, `FloatArray::toTypedArray`, `DoubleArray::toTypedArray`, `BooleanArray::toTypedArray`, `CharArray::toTypedArray`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 33. llvm.IrToBitcode

- **Target:** `kotlinc_native_ref.IrToBitcode_coroutines`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 2091010.0
- **Functions:** 1/184 matched (target 7)
- **Missing functions:** `NativeCodeGeneratorException::wrap`, `IrSimpleFunction::shouldGenerateBody`, `RTTIGeneratorVisitor::visitElement`, `RTTIGeneratorVisitor::visitClass`, `RTTIGeneratorVisitor::dispose`, `CodeContext::onEnter`, `CodeContext::onExit`, `CodeGeneratorVisitor::calculateLifetime`, `CodeGeneratorVisitor::evaluateExplicitArgs`, `CodeGeneratorVisitor::evaluateExpression`, `CodeGeneratorVisitor::getObjectFieldPointer`, `CodeGeneratorVisitor::getStaticFieldPointer`, `CodeGeneratorVisitor::TopLevelCodeContext::unsupported`, `CodeGeneratorVisitor::TopLevelCodeContext::genReturn`, `CodeGeneratorVisitor::TopLevelCodeContext::getReturnSlot`, `CodeGeneratorVisitor::TopLevelCodeContext::genBreak`, `CodeGeneratorVisitor::TopLevelCodeContext::genContinue`, `CodeGeneratorVisitor::TopLevelCodeContext::genDeclareVariable`, `CodeGeneratorVisitor::TopLevelCodeContext::getDeclaredValue`, `CodeGeneratorVisitor::TopLevelCodeContext::genGetValue`, `CodeGeneratorVisitor::TopLevelCodeContext::functionScope`, `CodeGeneratorVisitor::TopLevelCodeContext::fileScope`, `CodeGeneratorVisitor::TopLevelCodeContext::classScope`, `CodeGeneratorVisitor::TopLevelCodeContext::addResumePoint`, `CodeGeneratorVisitor::TopLevelCodeContext::returnableBlockScope`, `CodeGeneratorVisitor::TopLevelCodeContext::location`, `CodeGeneratorVisitor::TopLevelCodeContext::scope`, `CodeGeneratorVisitor::TopLevelCodeContext::wrapException`, `CodeGeneratorVisitor::using`, `CodeGeneratorVisitor::appendCAdapters`, `CodeGeneratorVisitor::runAndProcessInitializers`, `CodeGeneratorVisitor::visitElement`, `CodeGeneratorVisitor::visitModuleFragment`, `CodeGeneratorVisitor::createInitBody`, `CodeGeneratorVisitor::mergeRuntimeInitializers`, `CodeGeneratorVisitor::generateRuntimeInitializer`, `CodeGeneratorVisitor::createInitNode`, `CodeGeneratorVisitor::createInitCtor`, `CodeGeneratorVisitor::visitFile`, `CodeGeneratorVisitor::StackLocalsScope::onEnter`, `CodeGeneratorVisitor::StackLocalsScope::onExit`, `CodeGeneratorVisitor::LoopScope::genBreak`, `CodeGeneratorVisitor::LoopScope::genContinue`, `CodeGeneratorVisitor::evaluateBreak`, `CodeGeneratorVisitor::evaluateContinue`, `CodeGeneratorVisitor::visitAnonymousInitializer`, `CodeGeneratorVisitor::VariableScope::genDeclareVariable`, `CodeGeneratorVisitor::VariableScope::getDeclaredValue`, `CodeGeneratorVisitor::VariableScope::genGetValue`, `CodeGeneratorVisitor::ParameterScope::genGetValue`, `CodeGeneratorVisitor::FunctionScope::genReturn`, `CodeGeneratorVisitor::FunctionScope::getReturnSlot`, `CodeGeneratorVisitor::FunctionScope::functionScope`, `CodeGeneratorVisitor::FunctionScope::location`, `CodeGeneratorVisitor::FunctionScope::scope`, `CodeGeneratorVisitor::FunctionScope::wrapException`, `CodeGeneratorVisitor::bindParameters`, `CodeGeneratorVisitor::getGlobalInitStateFor`, `CodeGeneratorVisitor::getThreadLocalInitStateFor`, `CodeGeneratorVisitor::buildVirtualFunctionTrampoline`, `CodeGeneratorVisitor::visitConstructor`, `CodeGeneratorVisitor::handleStaticInitializer`, `CodeGeneratorVisitor::visitSimpleFunction`, `CodeGeneratorVisitor::location`, `CodeGeneratorVisitor::visitClass`, `CodeGeneratorVisitor::visitProperty`, `CodeGeneratorVisitor::visitField`, `CodeGeneratorVisitor::evaluateExpression`, `CodeGeneratorVisitor::generateStatement`, `CodeGeneratorVisitor::generate`, `CodeGeneratorVisitor::evaluateExpressionAndJump`, `CodeGeneratorVisitor::jump`, `CodeGeneratorVisitor::continuationBlock`, `CodeGeneratorVisitor::evaluateVararg`, `CodeGeneratorVisitor::evaluateThrow`, `CodeGeneratorVisitor::CatchingScope::endLocationInfoFromScope`, `CodeGeneratorVisitor::CatchingScope::jumpToHandler`, `CodeGeneratorVisitor::CatchingScope::genLandingpad`, `CodeGeneratorVisitor::CatchingScope::genThrow`, `CodeGeneratorVisitor::CatchScope::genHandler`, `CodeGeneratorVisitor::CatchScope::genCatchBlock`, `CodeGeneratorVisitor::evaluateTry`, `CodeGeneratorVisitor::evaluateWhen`, `CodeGeneratorVisitor::generateDebugTrambolineIf`, `CodeGeneratorVisitor::generateWhenCase`, `CodeGeneratorVisitor::evaluateWhileLoop`, `CodeGeneratorVisitor::evaluateDoWhileLoop`, `CodeGeneratorVisitor::evaluateGetValue`, `CodeGeneratorVisitor::evaluateSetValue`, `CodeGeneratorVisitor::debugInfoIfNeeded`, `CodeGeneratorVisitor::shouldGenerateDebugInfo`, `CodeGeneratorVisitor::generateVariable`, `CodeGeneratorVisitor::genDeclareVariable`, `CodeGeneratorVisitor::evaluateTypeOperator`, `CodeGeneratorVisitor::isPrimitiveInteger`, `CodeGeneratorVisitor::isUnsignedInteger`, `CodeGeneratorVisitor::evaluateIntegerCoercion`, `CodeGeneratorVisitor::evaluateCast`, `CodeGeneratorVisitor::evaluateInstanceOf`, `CodeGeneratorVisitor::genInstanceOf`, `CodeGeneratorVisitor::genInstanceOfImpl`, `CodeGeneratorVisitor::genInstanceOfObjC`, `CodeGeneratorVisitor::genInstanceOfObjCProtocol`, `CodeGeneratorVisitor::genInstanceOfProtocolViaProtocolGetter`, `CodeGeneratorVisitor::genInstanceOfObjCProtocolByName`, `CodeGeneratorVisitor::evaluateNotInstanceOf`, `CodeGeneratorVisitor::evaluateGetField`, `CodeGeneratorVisitor::isZeroConstValue`, `CodeGeneratorVisitor::evaluateSetField`, `CodeGeneratorVisitor::fieldPtrOfClass`, `CodeGeneratorVisitor::staticFieldPtr`, `CodeGeneratorVisitor::evaluateStringConst`, `CodeGeneratorVisitor::normalizeNan`, `CodeGeneratorVisitor::normalizeNan`, `CodeGeneratorVisitor::evaluateConst`, `CodeGeneratorVisitor::IrConstValueCacheKey::equals`, `CodeGeneratorVisitor::IrConstValueCacheKey::hashCode`, `CodeGeneratorVisitor::evaluateConstantValue`, `CodeGeneratorVisitor::evaluateConstantValueImpl`, `CodeGeneratorVisitor::evaluateReturn`, `CodeGeneratorVisitor::InlinedBlockScope::location`, `CodeGeneratorVisitor::InlinedBlockScope::scope`, `CodeGeneratorVisitor::InlinedBlockScope::wrapException`, `CodeGeneratorVisitor::ReturnableBlockScope::getExit`, `CodeGeneratorVisitor::ReturnableBlockScope::getResult`, `CodeGeneratorVisitor::ReturnableBlockScope::genReturn`, `CodeGeneratorVisitor::ReturnableBlockScope::getReturnSlot`, `CodeGeneratorVisitor::ReturnableBlockScope::returnableBlockScope`, `CodeGeneratorVisitor::usingFileScope`, `CodeGeneratorVisitor::FileScope::fileScope`, `CodeGeneratorVisitor::FileScope::location`, `CodeGeneratorVisitor::FileScope::scope`, `CodeGeneratorVisitor::FileScope::wrapException`, `CodeGeneratorVisitor::ClassScope::classScope`, `CodeGeneratorVisitor::ClassScope::wrapException`, `CodeGeneratorVisitor::evaluateReturnableBlock`, `CodeGeneratorVisitor::evaluateInlinedBlock`, `CodeGeneratorVisitor::evaluateContainerExpression`, `CodeGeneratorVisitor::evaluateInstanceInitializerCall`, `CodeGeneratorVisitor::evaluateCall`, `CodeGeneratorVisitor::fileEntry`, `CodeGeneratorVisitor::updateBuilderDebugLocation`, `CodeGeneratorVisitor::startLine`, `CodeGeneratorVisitor::startLineAndColumn`, `CodeGeneratorVisitor::endLineAndColumn`, `CodeGeneratorVisitor::debugFieldDeclaration`, `CodeGeneratorVisitor::diFileScope`, `CodeGeneratorVisitor::scope`, `CodeGeneratorVisitor::scope`, `CodeGeneratorVisitor::returnsUnit`, `CodeGeneratorVisitor::evaluateExplicitArgs`, `CodeGeneratorVisitor::evaluateRawFunctionReference`, `CodeGeneratorVisitor::evaluateSuspendableExpression`, `CodeGeneratorVisitor::SuspensionPointScope::genGetValue`, `CodeGeneratorVisitor::evaluateSuspensionPoint`, `CodeGeneratorVisitor::evaluateClassReference`, `CodeGeneratorVisitor::evaluateFunctionCall`, `CodeGeneratorVisitor::evaluateFileGlobalInitializerCall`, `CodeGeneratorVisitor::evaluateFileThreadLocalInitializerCall`, `CodeGeneratorVisitor::evaluateFileStandaloneThreadLocalInitializerCall`, `CodeGeneratorVisitor::evaluateSimpleFunctionCall`, `CodeGeneratorVisitor::resultLifetime`, `CodeGeneratorVisitor::genGetObjCClass`, `CodeGeneratorVisitor::genGetObjCProtocol`, `CodeGeneratorVisitor::evaluateOperatorCall`, `CodeGeneratorVisitor::callDirect`, `CodeGeneratorVisitor::callVirtual`, `CodeGeneratorVisitor::call`, `CodeGeneratorVisitor::call`, `CodeGeneratorVisitor::appendLlvmUsed`, `CodeGeneratorVisitor::overrideRuntimeGlobal`, `CodeGeneratorVisitor::overrideRuntimeGlobals`, `CodeGeneratorVisitor::createGlobalCtor`, `CodeGeneratorVisitor::appendStaticInitializers`, `CodeGeneratorVisitor::fileCtorName`, `CodeGeneratorVisitor::ctorProto`, `CodeGeneratorVisitor::appendStaticInitializers`, `CodeGeneratorVisitor::appendGlobalCtors`, `CodeGeneratorVisitor::basicBlock`, `IrValueDeclaration::debugNameConversion`, `setRuntimeConstGlobal`, `Map<LoggingTag, LoggingLevel>::toLLVMConstArray`, `NativeGenerationState::generateRuntimeConstantsModule`
- **Types:** 2/26 matched (target 3)
- **Missing types:** `NativeCodeGeneratorException`, `FieldStorageKind`, `RTTIGeneratorVisitor`, `CodeContext`, `CodeGeneratorVisitor`, `TopLevelCodeContext`, `InnerScope`, `InnerScopeImpl`, `StackLocalsScope`, `LoopScope`, `VariableScope`, `ParameterScope`, `FunctionScope`, `CatchingScope`, `CatchScope`, `WhenEmittingContext`, `BranchCaseNextInfo`, `IrConstValueCacheKey`, `InlinedBlockScope`, `ReturnableBlockScope`, `FileScope`, `ClassScope`, `SuspensionPointScope`, `LocationInfo`
- **Lint issues:** 1

### 34. declarations.IrTypeParameter

- **Target:** `declarations.IrTypeParameter`
- **Similarity:** 0.21
- **Dependents:** 2
- **Priority Score:** 2010307.9
- **Functions:** 1/2 matched (target 4)
- **Missing functions:** `IrTypeParameter::accept`
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 35. collections.Set

- **Target:** `collections.Set [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 2
- **Priority Score:** 2000210.0
- **Functions:** 0/0 matched (target 9)
- **Missing functions:** _none_
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_

### 36. declarations.IrValueDeclaration

- **Target:** `declarations.IrValueDeclaration [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 2
- **Priority Score:** 2000110.0
- **Functions:** 0/0 matched (target 1)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 37. time.Duration

- **Target:** `time.Duration`
- **Similarity:** 0.20
- **Dependents:** 1
- **Priority Score:** 1263708.0
- **Functions:** 11/37 matched (target 42)
- **Missing functions:** `convert`, `parse`, `parseIsoString`, `parseOrNull`, `parseIsoStringOrNull`, `unaryMinus`, `plus`, `minus`, `times`, `times`, `div`, `div`, `div`, `toComponents`, `toComponents`, `toComponents`, `toComponents`, `toDouble`, `toInt`, `toString`, `StringBuilder::appendFractional`, `toString`, `toIsoString`, `parse`, `parse`, `String::parseDigits`
- **Types:** 0/0 matched (target 2)
- **Missing types:** _none_
- **Lint issues:** 1

### 38. lower.UpgradeCallableReferences

- **Target:** `clang_suspend_plugin.UpgradeCallableReferences`
- **Similarity:** 0.01
- **Dependents:** 1
- **Priority Score:** 1222709.9
- **Functions:** 3/23 matched (target 4)
- **Missing functions:** `UpgradeCallableReferences::lower`, `UpgradeCallableReferences::getSamConversionArgument`, `UpgradeCallableReferences::UpgradeTransformer::visitElement`, `UpgradeCallableReferences::UpgradeTransformer::visitDeclaration`, `UpgradeCallableReferences::UpgradeTransformer::visitFile`, `UpgradeCallableReferences::UpgradeTransformer::arrayDepth`, `UpgradeCallableReferences::UpgradeTransformer::hasVarargConversion`, `UpgradeCallableReferences::UpgradeTransformer::parseAdaptedBlock`, `UpgradeCallableReferences::UpgradeTransformer::visitBlock`, `UpgradeCallableReferences::UpgradeTransformer::visitTypeOperator`, `UpgradeCallableReferences::UpgradeTransformer::getCapturedValues`, `UpgradeCallableReferences::UpgradeTransformer::visitFunctionReference`, `UpgradeCallableReferences::UpgradeTransformer::visitPropertyReference`, `UpgradeCallableReferences::UpgradeTransformer::fixCallableReferenceComingFromKlib`, `UpgradeCallableReferences::UpgradeTransformer::visitLocalDelegatedPropertyReference`, `UpgradeCallableReferences::UpgradeTransformer::buildAccessorFunctionForLocalDelegatedProperty`, `UpgradeCallableReferences::UpgradeTransformer::buildWrapperFunction`, `UpgradeCallableReferences::UpgradeTransformer::wrapField`, `UpgradeCallableReferences::UpgradeTransformer::wrapFunction`, `UpgradeCallableReferences::UpgradeTransformer::orderParametersToForward`
- **Types:** 2/4 matched (target 3)
- **Missing types:** `AdaptedBlock`, `CapturedValue`
- **Lint issues:** 2

### 39. lower.AbstractFunctionReferenceLowering

- **Target:** `clang_suspend_plugin.AbstractFunctionReferenceLowering`
- **Similarity:** 0.00
- **Dependents:** 1
- **Priority Score:** 1172010.0
- **Functions:** 2/19 matched (target 8)
- **Missing functions:** `AbstractFunctionReferenceLowering::lower`, `AbstractFunctionReferenceLowering::visitClass`, `AbstractFunctionReferenceLowering::visitBody`, `AbstractFunctionReferenceLowering::visitDeclaration`, `AbstractFunctionReferenceLowering::visitRichFunctionReference`, `AbstractFunctionReferenceLowering::visitFunctionReference`, `AbstractFunctionReferenceLowering::visitReturn`, `AbstractFunctionReferenceLowering::visitDeclaration`, `AbstractFunctionReferenceLowering::postprocessClass`, `AbstractFunctionReferenceLowering::postprocessInvoke`, `AbstractFunctionReferenceLowering::generateExtraMethods`, `AbstractFunctionReferenceLowering::getExtraConstructorParameters`, `AbstractFunctionReferenceLowering::getExtraConstructorArgument`, `AbstractFunctionReferenceLowering::getAdditionalInterfaces`, `CommonBackendContext::addBoundValueAtOverride`, `IrBuilderWithScope::irBoundValueAt`, `boundValue`
- **Types:** 1/1 matched (target 5)
- **Missing types:** _none_
- **Lint issues:** 2

### 40. expressions.IrSuspendableExpression

- **Target:** `expressions.IrSuspendableExpression`
- **Similarity:** 0.00
- **Dependents:** 1
- **Priority Score:** 1030410.0
- **Functions:** 0/3 matched
- **Missing functions:** `IrSuspendableExpression::accept`, `IrSuspendableExpression::acceptChildren`, `IrSuspendableExpression::transformChildren`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 41. descriptors.ValueParameterDescriptor

- **Target:** `descriptors.ValueParameterDescriptor`
- **Similarity:** 0.99
- **Dependents:** 1
- **Priority Score:** 1000200.1
- **Functions:** 1/1 matched (target 2)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 42. impl.IrSuspendableExpressionImpl

- **Target:** `impl.IrSuspendableExpressionImpl [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 1
- **Priority Score:** 1000110.0
- **Functions:** 0/0 matched (target 13)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 43. impl.IrSuspensionPointImpl

- **Target:** `impl.IrSuspensionPointImpl [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 1
- **Priority Score:** 1000110.0
- **Functions:** 0/0 matched (target 15)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 44. impl.IrSetValueImpl

- **Target:** `impl.IrSetValueImpl [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 1
- **Priority Score:** 1000110.0
- **Functions:** 0/0 matched (target 15)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 45. declarations.IrDeclarationParent

- **Target:** `declarations.IrDeclarationParent`
- **Similarity:** 1.00
- **Dependents:** 1
- **Priority Score:** 1000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 46. mpp.TypeRefMarker

- **Target:** `mpp.TypeRefMarker`
- **Similarity:** 1.00
- **Dependents:** 1
- **Priority Score:** 1000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 47. reflect.KType

- **Target:** `reflect.KType`
- **Similarity:** 1.00
- **Dependents:** 1
- **Priority Score:** 1000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_

### 48. model.TypeSystemContext

- **Target:** `model.TypeSystemContext [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 980610.0
- **Functions:** 0/74 matched (target 0)
- **Missing functions:** `TypeVariance::toString`, `Variance::convertVariance`, `TypeVariance::convertVariance`, `TypeSystemOptimizationContext::identicalArguments`, `TypeSystemTypeFactoryContext::createTrivialFlexibleTypeOrSelf`, `TypeSystemTypeFactoryContext::isTriviallyFlexible`, `TypeSystemTypeFactoryContext::makeLowerBoundDefinitelyNotNullOrNotNull`, `TypeCheckerProviderContext::newTypeCheckerState`, `TypeSystemCommonSuperTypesContext::anySuperTypeConstructor`, `TypeSystemCommonSuperTypesContext::typeDepth`, `TypeSystemCommonSuperTypesContext::typeDepthForApproximation`, `TypeSystemInferenceExtensionContext::replaceArguments`, `TypeSystemInferenceExtensionContext::replaceArgumentsDeeply`, `TypeSystemInferenceExtensionContext::replaceArgumentsDeeply`, `TypeSystemInferenceExtensionContext::getUpperBoundForApproximationOfIntersectionType`, `TypeSystemInferenceExtensionContext::extractTypeOf`, `TypeSystemInferenceExtensionContext::extractTypeVariables`, `TypeSystemInferenceExtensionContext::extractTypeParameters`, `TypeSystemInferenceExtensionContext::createCapturedStarProjectionForSelfType`, `TypeSystemInferenceExtensionContext::computeEmptyIntersectionTypeKind`, `TypeSystemContext::asRigidType`, `TypeSystemContext::asFlexibleType`, `TypeSystemContext::asDynamicType`, `TypeSystemContext::asCapturedType`, `TypeSystemContext::asCapturedTypeUnwrappingDnn`, `TypeSystemContext::asCapturedTypeUnwrappingDnn`, `TypeSystemContext::isCapturedType`, `TypeSystemContext::asDefinitelyNotNullType`, `TypeSystemContext::originalIfDefinitelyNotNullable`, `TypeSystemContext::originalIfDefinitelyNotNullable`, `TypeSystemContext::makeDefinitelyNotNullOrNotNull`, `TypeSystemContext::makeDefinitelyNotNullOrNotNull`, `TypeSystemContext::getArgumentOrNull`, `TypeSystemContext::isUnion`, `TypeSystemContext::getPrimaryTypeOfUnion`, `TypeSystemContext::getRichErrorsOfUnion`, `TypeSystemContext::lowerBoundIfFlexible`, `TypeSystemContext::lowerBoundIfFlexible`, `TypeSystemContext::upperBoundIfFlexible`, `TypeSystemContext::upperBoundIfFlexible`, `TypeSystemContext::isFlexibleWithDifferentTypeConstructors`, `TypeSystemContext::isFlexible`, `TypeSystemContext::isDynamic`, `TypeSystemContext::isCapturedDynamic`, `TypeSystemContext::isDefinitelyNotNullType`, `TypeSystemContext::isDefinitelyNotNullType`, `TypeSystemContext::isNotNullTypeParameter`, `TypeSystemContext::hasFlexibleNullability`, `TypeSystemContext::typeConstructor`, `TypeSystemContext::isNullableType`, `TypeSystemContext::isNullableAny`, `TypeSystemContext::isNothing`, `TypeSystemContext::isFlexibleNothing`, `TypeSystemContext::isNullableNothing`, `TypeSystemContext::isClassType`, `TypeSystemContext::fastCorrespondingSupertypes`, `TypeSystemContext::isIntegerLiteralType`, `TypeSystemContext::get`, `TypeSystemContext::size`, `TypeSystemContext::iterator`, `TypeSystemContext::hasNext`, `TypeSystemContext::next`, `TypeSystemContext::isAnyConstructor`, `TypeSystemContext::isNonErrorConstructor`, `TypeSystemContext::isRichErrorConstructor`, `TypeSystemContext::isNothingConstructor`, `TypeSystemContext::isArrayConstructor`, `TypeSystemContext::isRichErrorClass`, `TypeSystemContext::withNewTypeSince`, `TypeSystemContext::isRigidType`, `TypeSystemContext::isPrimitiveType`, `TypeSystemContext::substituteOrNull`, `TypeArgumentListMarker::all`, `requireOrDescribe`
- **Types:** 9/32 matched (target 9)
- **Missing types:** `DefinitelyNotNullTypeMarker`, `CapturedTypeMarker`, `StubTypeMarker`, `TypeVariableMarker`, `TypeVariableTypeConstructorMarker`, `CapturedTypeConstructorMarker`, `IntersectionTypeConstructorMarker`, `TypeSubstitutorMarker`, `AnnotationMarker`, `TypeVariance`, `TypeSystemOptimizationContext`, `TypeSystemBuiltInsContext`, `TypeSystemTypeFactoryContext`, `TypeCheckerProviderContext`, `TypeSystemCommonSuperTypesContext`, `TypeSystemInferenceExtensionContextDelegate`, `TypeSystemInferenceExtensionContext`, `ArgumentList`, `TypeSystemContext`, `CustomSubtypingCallback`, `CaptureStatus`, `ObsoleteTypeKind`, `K2Only`

### 49. collections.Collections

- **Target:** `collections.CollectionToArray`
- **Similarity:** 0.01
- **Dependents:** 0
- **Priority Score:** 555709.9
- **Functions:** 2/54 matched (target 2)
- **Missing functions:** `EmptyIterator::hasNext`, `EmptyIterator::hasPrevious`, `EmptyIterator::nextIndex`, `EmptyIterator::previousIndex`, `EmptyIterator::next`, `EmptyIterator::previous`, `EmptyList::equals`, `EmptyList::hashCode`, `EmptyList::toString`, `EmptyList::isEmpty`, `EmptyList::contains`, `EmptyList::containsAll`, `EmptyList::get`, `EmptyList::indexOf`, `EmptyList::lastIndexOf`, `EmptyList::iterator`, `EmptyList::listIterator`, `EmptyList::listIterator`, `EmptyList::subList`, `EmptyList::readResolve`, `Array<out T>::asCollection`, `ArrayAsCollection::isEmpty`, `ArrayAsCollection::contains`, `ArrayAsCollection::containsAll`, `ArrayAsCollection::iterator`, `ArrayAsCollection::toArray`, `emptyList`, `listOf`, `listOf`, `mutableListOf`, `arrayListOf`, `mutableListOf`, `arrayListOf`, `listOfNotNull`, `listOfNotNull`, `List`, `MutableList`, `buildList`, `buildList`, `Collection<T>::isNotEmpty`, `isNullOrEmpty`, `orEmpty`, `orEmpty`, `C::ifEmpty`, `Collection<T>::containsAll`, `Iterable<T>::shuffled`, `List<T>::optimizeReadOnlyList`, `List<T?>::binarySearch`, `List<T>::binarySearch`, `List<T>::binarySearchBy`, `List<T>::binarySearch`, `rangeCheck`
- **Types:** 0/3 matched (target 0)
- **Missing types:** `EmptyIterator`, `EmptyList`, `ArrayAsCollection`

### 50. atomics.Atomics.native

- **Target:** `atomics.Atomics`
- **Similarity:** 0.10
- **Dependents:** 0
- **Priority Score:** 486709.0
- **Functions:** 18/62 matched (target 28)
- **Missing functions:** `AtomicLong::load`, `AtomicLong::store`, `AtomicLong::exchange`, `AtomicLong::compareAndSet`, `AtomicLong::compareAndExchange`, `AtomicLong::fetchAndAdd`, `AtomicLong::addAndFetch`, `AtomicLong::getAndSet`, `AtomicLong::getAndAdd`, `AtomicLong::addAndGet`, `AtomicLong::getAndIncrement`, `AtomicLong::incrementAndGet`, `AtomicLong::decrementAndGet`, `AtomicLong::getAndDecrement`, `AtomicLong::toString`, `AtomicBoolean::load`, `AtomicBoolean::store`, `AtomicBoolean::exchange`, `AtomicBoolean::compareAndSet`, `AtomicBoolean::compareAndExchange`, `AtomicBoolean::toString`, `AtomicReference::load`, `AtomicReference::store`, `AtomicReference::exchange`, `AtomicReference::compareAndSet`, `AtomicReference::compareAndExchange`, `AtomicReference::getAndSet`, `AtomicReference::toString`, `AtomicNativePtr::load`, `AtomicNativePtr::store`, `AtomicNativePtr::exchange`, `AtomicNativePtr::compareAndSet`, `AtomicNativePtr::compareAndExchange`, `AtomicNativePtr::getAndSet`, `AtomicNativePtr::toString`, `AtomicLong::update`, `AtomicLong::fetchAndUpdate`, `AtomicLong::updateAndFetch`, `AtomicReference<T>::update`, `AtomicReference<T>::fetchAndUpdate`, `AtomicReference<T>::updateAndFetch`, `AtomicNativePtr::update`, `AtomicNativePtr::fetchAndUpdate`, `AtomicNativePtr::updateAndFetch`
- **Types:** 1/5 matched (target 1)
- **Missing types:** `AtomicLong`, `AtomicBoolean`, `AtomicReference`, `AtomicNativePtr`
- **Lint issues:** 1

### 51. util.IrTypeUtils

- **Target:** `clang_suspend_plugin.IrTypeUtils`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 464710.0
- **Functions:** 1/47 matched (target 2)
- **Missing functions:** `IrType::isFunctionMarker`, `IrType::isFunction`, `IrType::isKFunction`, `IrType::isSuspendFunction`, `IrType::isKSuspendFunction`, `IrType::isKProperty`, `IrType::isKMutableProperty`, `IrClassifierSymbol::isFunctionMarker`, `IrClassifierSymbol::isFunction`, `IrClassifierSymbol::isKFunction`, `IrClassifierSymbol::isSuspendFunction`, `IrClassifierSymbol::isKSuspendFunction`, `IrClassifierSymbol::isFunctional`, `IrClassifierSymbol::isClassWithName`, `IrClassifierSymbol::isClassWithNamePrefix`, `IrClassifierSymbol::checkNameAndPackage`, `IrClassifierSymbol::superTypes`, `IrClassifierSymbol::isSubtypeOfClass`, `IrClassifierSymbol::isStrictSubtypeOfClass`, `IrType::superTypes`, `IrType::isFunctionTypeOrSubtype`, `IrType::isSuspendFunctionTypeOrSubtype`, `IrType::isTypeParameter`, `IrType::isInterface`, `IrType::isExternalObject`, `IrType::isAnnotation`, `IrType::isFunctionOrKFunction`, `IrType::isSuspendFunctionOrKFunction`, `IrType::isThrowable`, `IrType::isUnsigned`, `IrType::isUnsignedArray`, `IrType::isTypeFromKotlinPackage`, `IrType::isPrimitiveArray`, `IrType::getPrimitiveArrayElementType`, `IrType::substitute`, `IrType::substitute`, `IrType::isSubtypeOfClass`, `IrType::isStrictSubtypeOfClass`, `IrType::isSubtypeOf`, `IrType::isNullable`, `IrType::getArrayElementType`, `IrType::toArrayOrPrimitiveArrayType`, `getImmediateSupertypes`, `collectAllSupertypes`, `getAllSubstitutedSupertypes`, `IrClass::getAllSuperclasses`
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_

### 52. kotlin.Arrays

- **Target:** `kotlin.IntArray`
- **Similarity:** 0.04
- **Dependents:** 0
- **Priority Score:** 354009.6
- **Functions:** 3/24 matched (target 13)
- **Missing functions:** `ByteArrayIterator::hasNext`, `ByteArrayIterator::nextByte`, `iterator`, `CharArrayIterator::hasNext`, `CharArrayIterator::nextChar`, `iterator`, `ShortArrayIterator::hasNext`, `ShortArrayIterator::nextShort`, `iterator`, `iterator`, `LongArrayIterator::hasNext`, `LongArrayIterator::nextLong`, `iterator`, `FloatArrayIterator::hasNext`, `FloatArrayIterator::nextFloat`, `iterator`, `DoubleArrayIterator::hasNext`, `DoubleArrayIterator::nextDouble`, `iterator`, `BooleanArrayIterator::hasNext`, `BooleanArrayIterator::nextBoolean`
- **Types:** 2/16 matched (target 3)
- **Missing types:** `ByteArray`, `ByteArrayIterator`, `CharArray`, `CharArrayIterator`, `ShortArray`, `ShortArrayIterator`, `LongArray`, `LongArrayIterator`, `FloatArray`, `FloatArrayIterator`, `DoubleArray`, `DoubleArrayIterator`, `BooleanArray`, `BooleanArrayIterator`
- **Lint issues:** 1

### 53. lower.NativeSuspendFunctionLowering

- **Target:** `clang_suspend_plugin.NativeSuspendLowering`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 293510.0
- **Functions:** 5/33 matched (target 41)
- **Missing functions:** `NativeSuspendFunctionsLowering::lower`, `NativeSuspendFunctionsLowering::visitElement`, `NativeSuspendFunctionsLowering::visitClass`, `NativeSuspendFunctionsLowering::tryTransformSuspendFunction`, `NativeSuspendFunctionsLowering::isReturnIfSuspendedCall`, `NativeSuspendFunctionsLowering::simplifyTailSuspendCalls`, `NativeSuspendFunctionsLowering::visitCall`, `NativeSuspendFunctionsLowering::generateCoroutineStart`, `NativeSuspendFunctionsLowering::visitReturn`, `NativeSuspendFunctionsLowering::visitGetValue`, `NativeSuspendFunctionsLowering::visitSetValue`, `NativeSuspendFunctionsLowering::ExpressionSlicer::visitSetField`, `NativeSuspendFunctionsLowering::ExpressionSlicer::visitMemberAccess`, `NativeSuspendFunctionsLowering::ExpressionSlicer::sliceExpression`, `NativeSuspendFunctionsLowering::ExpressionSlicer::saveState`, `NativeSuspendFunctionsLowering::irWrap`, `NativeSuspendFunctionsLowering::irThrowIfNotNull`, `NativeSuspendFunctionsLowering::irThrowIfNotNull`, `NativeSuspendFunctionsLowering::isSpecialBlock`, `NativeSuspendFunctionsLowering::visitElement`, `NativeSuspendFunctionsLowering::visitCall`, `NativeSuspendFunctionsLowering::visitExpression`, `NativeSuspendFunctionsLowering::irVar`, `NativeSuspendFunctionsLowering::irReturnIfSuspended`, `NativeSuspendFunctionsLowering::irVar`, `NativeSuspendFunctionsLowering::irGetOrThrow`, `NativeSuspendFunctionsLowering::irExceptionOrNull`, `NativeSuspendFunctionsLowering::irSuccess`
- **Types:** 1/2 matched (target 10)
- **Missing types:** `ExpressionSlicer`
- **Lint issues:** 4

### 54. collections.MutableCollections

- **Target:** `collections.MutableCollections`
- **Similarity:** 0.06
- **Dependents:** 0
- **Priority Score:** 273309.4
- **Functions:** 6/33 matched (target 6)
- **Missing functions:** `MutableCollection<out T>::remove`, `MutableCollection<out T>::removeAll`, `MutableCollection<out T>::retainAll`, `MutableCollection<in T>::plusAssign`, `MutableCollection<in T>::plusAssign`, `MutableCollection<in T>::plusAssign`, `MutableCollection<in T>::plusAssign`, `MutableCollection<in T>::minusAssign`, `MutableCollection<in T>::minusAssign`, `MutableCollection<in T>::minusAssign`, `MutableCollection<in T>::minusAssign`, `MutableCollection<in T>::addAll`, `MutableCollection<in T>::addAll`, `MutableCollection<in T>::addAll`, `Iterable<T>::convertToListIfNotCollection`, `MutableCollection<in T>::removeAll`, `MutableCollection<in T>::removeAll`, `MutableCollection<in T>::removeAll`, `MutableCollection<in T>::retainAll`, `MutableCollection<in T>::retainAll`, `MutableCollection<in T>::retainAll`, `MutableCollection<*>::retainNothing`, `MutableList<T>::remove`, `MutableList<T>::removeFirst`, `MutableList<T>::removeFirstOrNull`, `MutableList<T>::removeLast`, `MutableList<T>::removeLastOrNull`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 55. annotations.Annotations

- **Target:** `annotations.Annotated [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 252710.0
- **Functions:** 0/22 matched (target 0)
- **Missing functions:** `Annotations::findAnnotation`, `Annotations::hasAnnotation`, `Annotations::getUseSiteTargetedAnnotations`, `Annotations::isEmpty`, `Annotations::findAnnotation`, `Annotations::iterator`, `Annotations::toString`, `Annotations::create`, `FilteredAnnotations::hasAnnotation`, `FilteredAnnotations::findAnnotation`, `FilteredAnnotations::iterator`, `FilteredAnnotations::isEmpty`, `FilteredAnnotations::shouldBeReturned`, `FilteredByPredicateAnnotations::isEmpty`, `FilteredByPredicateAnnotations::iterator`, `FilteredByPredicateAnnotations::findAnnotation`, `CompositeAnnotations::isEmpty`, `CompositeAnnotations::hasAnnotation`, `CompositeAnnotations::findAnnotation`, `CompositeAnnotations::getUseSiteTargetedAnnotations`, `CompositeAnnotations::iterator`, `composeAnnotations`
- **Types:** 2/5 matched (target 2)
- **Missing types:** `FilteredAnnotations`, `FilteredByPredicateAnnotations`, `CompositeAnnotations`

### 56. lower.NativeFunctionReferenceLowering

- **Target:** `clang_suspend_plugin.NativeFunctionReferenceLowering`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 192110.0
- **Functions:** 1/19 matched (target 4)
- **Missing functions:** `NativeFunctionReferenceLowering::isLoweredFunctionReference`, `NativeFunctionReferenceLowering::postprocessClass`, `NativeFunctionReferenceLowering::getReferenceClassName`, `NativeFunctionReferenceLowering::getSuperClassType`, `NativeFunctionReferenceLowering::getClassOrigin`, `NativeFunctionReferenceLowering::getConstructorOrigin`, `NativeFunctionReferenceLowering::getInvokeMethodOrigin`, `NativeFunctionReferenceLowering::getConstructorCallOrigin`, `NativeFunctionReferenceLowering::generateSuperClassConstructorCall`, `NativeFunctionReferenceLowering::generateExtraMethods`, `NativeFunctionReferenceLowering::irKFunctionDescription`, `NativeFunctionReferenceLowering::KFunctionDescription::getFlags`, `NativeFunctionReferenceLowering::KFunctionDescription::getFqName`, `NativeFunctionReferenceLowering::KFunctionDescription::getName`, `NativeFunctionReferenceLowering::KFunctionDescription::getArity`, `NativeFunctionReferenceLowering::KFunctionDescription::getBoundValueCount`, `NativeFunctionReferenceLowering::KFunctionDescription::returnType`, `NativeFunctionReferenceLowering::KFunctionDescription::isFunInterfaceConstructorAdapter`
- **Types:** 1/2 matched (target 5)
- **Missing types:** `KFunctionDescription`
- **Lint issues:** 1

### 57. text.StringNumberConversions

- **Target:** `text.StringNumberConversions`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 171710.0
- **Functions:** 0/17 matched (target 5)
- **Missing functions:** `Byte::toString`, `Short::toString`, `Int::toString`, `Long::toString`, `toBoolean`, `String::toByte`, `String::toByte`, `String::toShort`, `String::toShort`, `String::toInt`, `String::toInt`, `String::toLong`, `String::toLong`, `String::toFloat`, `String::toDouble`, `String::toFloatOrNull`, `String::toDoubleOrNull`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 58. impl.IrSymbolImpl

- **Target:** `impl.IrVariableSymbolImpl [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 141710.0
- **Functions:** 0/0 matched (target 9)
- **Missing functions:** _none_
- **Types:** 3/17 matched (target 3)
- **Missing types:** `IrFileSymbolImpl`, `IrExternalPackageFragmentSymbolImpl`, `IrAnonymousInitializerSymbolImpl`, `IrEnumEntrySymbolImpl`, `IrFieldSymbolImpl`, `IrClassSymbolImpl`, `IrScriptSymbolImpl`, `IrReplSnippetSymbolImpl`, `IrConstructorSymbolImpl`, `IrSimpleFunctionSymbolImpl`, `IrReturnableBlockSymbolImpl`, `IrPropertySymbolImpl`, `IrLocalDelegatedPropertySymbolImpl`, `IrTypeAliasSymbolImpl`

### 59. expression.FirSuspendCallChecker

- **Target:** `clang_suspend_plugin.FirSuspendCallChecker`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 111609.8
- **Functions:** 4/12 matched (target 4)
- **Missing functions:** `FirSuspendCallChecker::checkSuspendModifierForm`, `FirSuspendCallChecker::formOfSuspendModifierForLambdaOrFun`, `FirSuspendCallChecker::checkRestrictsSuspension`, `FirSuspendCallChecker::isCaseMissedByK1`, `FirSuspendCallChecker::sameInstanceOfReceiver`, `FirSuspendCallChecker::computeReceiversInfo`, `FirSuspendCallChecker::zipReceiverInfo`, `FirSuspendCallChecker::checkCallableReference`
- **Types:** 1/4 matched (target 5)
- **Missing types:** `ReceiversInfo`, `ReceiverInfo`, `SuspendCallArgumentKind`

### 60. libraries.stdlib.native-wasm.kotlin.collections.Collections

- **Target:** `collections.Collections`
- **Similarity:** 0.08
- **Dependents:** 0
- **Priority Score:** 111309.2
- **Functions:** 1/11 matched (target 15)
- **Missing functions:** `Array<out T>::copyToArrayOfAny`, `buildListInternal`, `buildListInternal`, `Grouping<T, K>::eachCount`, `MutableList<T>::fill`, `MutableList<T>::shuffle`, `Iterable<T>::shuffled`, `checkCountOverflow`, `listOf`, `Array<out T>::asArrayList`
- **Types:** 1/2 matched (target 6)
- **Missing types:** `MutableIterable`

### 61. internal.KClassImpl

- **Target:** `internal.KClassImpl`
- **Similarity:** 0.10
- **Dependents:** 0
- **Priority Score:** 81309.0
- **Functions:** 4/11 matched (target 10)
- **Missing functions:** `KClass<*>::findAssociatedObject`, `KClassUnsupportedImpl::isInstance`, `KClassUnsupportedImpl::equals`, `KClassUnsupportedImpl::hashCode`, `KClassUnsupportedImpl::toString`, `checkNotNull`, `downcast`
- **Types:** 1/2 matched (target 1)
- **Missing types:** `KClassUnsupportedImpl`
- **Lint issues:** 1

### 62. util.transform

- **Target:** `util.TransformInPlace`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 80910.0
- **Functions:** 1/9 matched (target 2)
- **Missing functions:** `MutableList<T>::transformInPlace`, `MutableList<T?>::transformInPlace`, `Array<T?>::transformInPlace`, `MutableList<T>::transformFlat`, `MutableList<T>::transformSubsetFlat`, `MutableList<T>::replaceInPlace`, `IrDeclarationContainer::transformDeclarationsFlat`, `List<T>::transformIfNeeded`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 63. kotlin.ArrayIntrinsics

- **Target:** `kotlin.ArrayIntrinsics`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 80910.0
- **Functions:** 1/9 matched (target 2)
- **Missing functions:** `doubleArrayOf`, `floatArrayOf`, `longArrayOf`, `intArrayOf`, `charArrayOf`, `shortArrayOf`, `byteArrayOf`, `booleanArrayOf`
- **Types:** 0/0 matched (target 3)
- **Missing types:** _none_

### 64. common.TailSuspendCallsCollector

- **Target:** `clang_suspend_plugin.TailSuspendCallsCollector`
- **Similarity:** 0.13
- **Dependents:** 0
- **Priority Score:** 61608.8
- **Functions:** 8/14 matched (target 13)
- **Missing functions:** `isTailReturn`, `visitExpressionBody`, `visitBlockBody`, `visitContainerExpression`, `IrExpression::isUnitRead`, `IrCall::isReturnIfSuspendedCall`
- **Types:** 2/2 matched (target 5)
- **Missing types:** _none_
- **Lint issues:** 1

### 65. native.Runtime

- **Target:** `native.Runtime`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50510.0
- **Functions:** 0/3 matched (target 1)
- **Missing functions:** `initRuntimeIfNeeded`, `setUnhandledExceptionHook`, `getUnhandledExceptionHook`
- **Types:** 0/2 matched (target 0)
- **Missing types:** `IncorrectDereferenceException`, `ReportUnhandledExceptionHook`

### 66. collections.Arrays

- **Target:** `collections.Arrays`
- **Similarity:** 0.26
- **Dependents:** 0
- **Priority Score:** 41007.4
- **Functions:** 6/10 matched (target 6)
- **Missing functions:** `orEmpty`, `checkCopyOfRangeArguments`, `Array<out T>::subarrayContentToString`, `contentDeepHashCodeImpl`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 67. visitors.Deprecated

- **Target:** `visitors.Deprecated`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 20300.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/3 matched (target 1)
- **Missing types:** `IrElementVisitor`, `IrElementVisitorVoid`

### 68. compiler.ir.ir.tree.org.jetbrains.kotlin.ir.symbols.IrSymbol

- **Target:** `symbols.IrSymbol [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10310.0
- **Functions:** 0/0 matched (target 5)
- **Missing functions:** _none_
- **Types:** 2/3 matched (target 5)
- **Missing types:** `UnsafeDuringIrConstructionAPI`

### 69. collections.Collection

- **Target:** `collections.MutableCollection [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10210.0
- **Functions:** 0/0 matched (target 7)
- **Missing functions:** _none_
- **Types:** 1/2 matched (target 3)
- **Missing types:** `Collection`

### 70. collections.List

- **Target:** `collections.MutableList [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10210.0
- **Functions:** 0/0 matched (target 13)
- **Missing functions:** _none_
- **Types:** 1/2 matched
- **Missing types:** `List`

### 71. common.RestrictSuspensionUtils

- **Target:** `clang_suspend_plugin.RestrictSuspensionUtils`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 10209.8
- **Functions:** 1/2 matched (target 3)
- **Missing functions:** `IrFunction::isRestrictedSuspensionFunction`
- **Types:** 0/0 matched (target 2)
- **Missing types:** _none_

### 72. descriptors.Visibilities

- **Target:** `descriptors.Visibilities`
- **Similarity:** 0.70
- **Dependents:** 0
- **Priority Score:** 2203.0
- **Functions:** 12/12 matched (target 23)
- **Missing functions:** _none_
- **Types:** 10/10 matched
- **Missing types:** _none_
- **Lint issues:** 9

### 73. collections.PrimitiveIterators

- **Target:** `collections.PrimitiveIterators`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 1600.0
- **Functions:** 8/8 matched (target 16)
- **Missing functions:** _none_
- **Types:** 8/8 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 74. mpp.DeclarationSymbolMarkers

- **Target:** `mpp.DeclarationSymbolMarkers`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 1500.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 15/15 matched
- **Missing types:** _none_

### 75. descriptors.Visibility

- **Target:** `descriptors.Visibility`
- **Similarity:** 0.53
- **Dependents:** 0
- **Priority Score:** 604.7
- **Functions:** 5/5 matched (target 10)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 3

### 76. compiler.ir.ir.tree.org.jetbrains.kotlin.ir.symbols.impl.IrSymbolImpl

- **Target:** `impl.IrSymbolImpl`
- **Similarity:** 0.14
- **Dependents:** 0
- **Priority Score:** 508.6
- **Functions:** 3/3 matched (target 14)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 77. kotlin.Array

- **Target:** `kotlin.Array`
- **Similarity:** 0.30
- **Dependents:** 0
- **Priority Score:** 507.0
- **Functions:** 3/3 matched (target 12)
- **Missing functions:** _none_
- **Types:** 2/2 matched (target 5)
- **Missing types:** _none_
- **Lint issues:** 1

### 78. collections.Map

- **Target:** `collections.Map [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 410.0
- **Functions:** 0/0 matched (target 24)
- **Missing functions:** _none_
- **Types:** 4/4 matched (target 17)
- **Missing types:** _none_
- **Lint issues:** 3

### 79. collections.Iterator

- **Target:** `collections.Iterator [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 410.0
- **Functions:** 0/0 matched (target 6)
- **Missing functions:** _none_
- **Types:** 4/4 matched (target 8)
- **Missing types:** _none_

### 80. collections.ArrayUtil

- **Target:** `collections.ArrayUtil`
- **Similarity:** 0.19
- **Dependents:** 0
- **Priority Score:** 408.1
- **Functions:** 4/4 matched (target 17)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_

### 81. kotlin.Any

- **Target:** `kotlin.Any`
- **Similarity:** 0.53
- **Dependents:** 0
- **Priority Score:** 404.7
- **Functions:** 3/3 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 82. descriptors.Substitutable

- **Target:** `descriptors.Substitutable [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 110.0
- **Functions:** 0/0 matched (target 1)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 83. impl.IrTypeParameterImpl

- **Target:** `impl.IrTypeParameterImpl [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 110.0
- **Functions:** 0/0 matched (target 24)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 84. internal.TypeInfoNames

- **Target:** `internal.TypeInfoNames [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 110.0
- **Functions:** 0/0 matched (target 8)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 4)
- **Missing types:** _none_
- **Lint issues:** 1

### 85. impl.IrValueParameterImpl

- **Target:** `impl.IrValueParameterImpl [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 110.0
- **Functions:** 0/0 matched (target 32)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 86. reflect.KCallable

- **Target:** `reflect.KCallable`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 87. internal.TypeInfoHolder

- **Target:** `internal.TypeInfoHolder`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 88. collections.RandomAccess

- **Target:** `collections.RandomAccess`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 89. reflect.KAnnotatedElement

- **Target:** `reflect.KAnnotatedElement`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 90. expressions.IrDeclarationReference

- **Target:** `expressions.IrDeclarationReference`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 91. declarations.IrMetadataSourceOwner

- **Target:** `declarations.IrMetadataSourceOwner`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 92. expressions.IrValueAccessExpression

- **Target:** `expressions.IrValueAccessExpression`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 93. expressions.IrVarargElement

- **Target:** `expressions.IrVarargElement`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 94. declarations.IrDeclarationWithVisibility

- **Target:** `declarations.IrDeclarationWithVisibility`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 95. declarations.IrMutableAnnotationContainer

- **Target:** `declarations.IrMutableAnnotationContainer`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 96. declarations.IrPossiblyExternalDeclaration

- **Target:** `declarations.IrPossiblyExternalDeclaration`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 97. reflect.KDeclarationContainer

- **Target:** `reflect.KDeclarationContainer`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 98. reflect.KClassifier

- **Target:** `reflect.KClassifier`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

## Success Criteria

For each file to be considered "complete":
- **Similarity ≥ 0.85** (Excellent threshold)
- All public APIs ported
- All tests ported
- Documentation ported
- port-lint header present

