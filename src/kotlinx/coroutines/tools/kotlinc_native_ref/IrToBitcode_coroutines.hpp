// Reference transliteration seed from Kotlin/Native IrToBitcode.kt.
//
// Source of truth (vendored snapshot):
//   tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt
//
// This header is NOT compiled by default; it exists to preserve the exact
// coroutine‑related LLVM lowering patterns that kotlinc emits, as a guide for
// the C++ suspend‑DSL compiler plugin.
//
// Kotlin line references (snapshot):
//   - ContinuationBlock / continuationBlock(): ~989‑1025
//   - evaluateSuspendableExpression(): ~2377‑2393
//   - evaluateSuspensionPoint(): ~2407‑2423
//   - indirectBr helper: CodeGenerator.kt ~1245‑1249

#pragma once

#include <vector>
#include <cstdint>

// Forward declarations matching LLVM C API types used by Kotlin/Native.
using LLVMValueRef = void*;
using LLVMBasicBlockRef = void*;

namespace kotlinc_native_ref {

// -----------------------------------------------------------------------------
// ContinuationBlock helper (IrToBitcode.kt continuationBlock)
// -----------------------------------------------------------------------------

struct ContinuationBlock {
    LLVMBasicBlockRef block;
    LLVMValueRef valuePhi; // nullptr if Unit

    LLVMValueRef value_or_unit(LLVMValueRef unitInstance) const {
        return valuePhi ? valuePhi : unitInstance;
    }
};

// Pseudocode signature. In Kotlin this lives on FunctionGenerationContext and
// creates a basic block with an optional phi of the given type.
//
// ContinuationBlock continuationBlock(IrType type, LocationInfo* loc, Fn code)
//
// The key semantic: "continuation_block" is a merge target receiving the value
// produced by either normal execution or resume execution.

// -----------------------------------------------------------------------------
// SuspendableExpressionScope / SuspensionPointScope
// -----------------------------------------------------------------------------

struct SuspendableExpressionScope {
    std::vector<LLVMBasicBlockRef>& resumePoints;
    explicit SuspendableExpressionScope(std::vector<LLVMBasicBlockRef>& rp) : resumePoints(rp) {}

    int addResumePoint(LLVMBasicBlockRef bbLabel) {
        int id = static_cast<int>(resumePoints.size());
        resumePoints.push_back(bbLabel);
        return id;
    }
};

// Kotlin's SuspensionPointScope overrides genGetValue so that
// the suspensionPointIdParameter reads as blockAddress(bbResume).
struct SuspensionPointScope {
    LLVMValueRef suspensionPointIdParameter;
    LLVMBasicBlockRef bbResume;
    int bbResumeId;
};

// -----------------------------------------------------------------------------
// evaluateSuspendableExpression (exact Kotlin shape)
// -----------------------------------------------------------------------------
//
// Kotlin:
//   val suspensionPointId = evaluateExpression(expression.suspensionPointId)
//   if (suspensionPointId == null) goto bbStart else goto bbDispatch
//   bbDispatch: indirectBr(suspensionPointId, resumePoints)
//
// In C++ plugin lowering terms:
//   if (_label == nullptr) start; else goto *_label;
//
inline LLVMValueRef evaluateSuspendableExpression(
    LLVMValueRef suspensionPointId,
    LLVMBasicBlockRef bbStart,
    LLVMBasicBlockRef bbDispatch,
    std::vector<LLVMBasicBlockRef>& resumePoints,
    LLVMValueRef (*evaluateResult)(LLVMValueRef /*resultSlot*/)
) {
    // Pseudocode only; actual implementation will be in the compiler plugin.
    (void)bbStart;
    (void)bbDispatch;
    (void)resumePoints;
    return evaluateResult(nullptr);
}

// -----------------------------------------------------------------------------
// evaluateSuspensionPoint (exact Kotlin shape)
// -----------------------------------------------------------------------------
//
// Kotlin:
//   val bbResume = basicBlock("resume")
//   val id = currentCodeContext.addResumePoint(bbResume)
//   continuationBlock { normalResult = evaluate(result); jump(normalResult)
//                       positionAtEnd(bbResume); resumeResult = evaluate(resumeResult); jump(resumeResult) }
//
inline LLVMValueRef evaluateSuspensionPoint(
    LLVMValueRef /*suspensionPointIdParameter*/,
    std::vector<LLVMBasicBlockRef>& resumePoints,
    LLVMValueRef (*evaluateNormal)(),
    LLVMValueRef (*evaluateResume)()
) {
    // Pseudocode only; actual implementation will be in the compiler plugin.
    (void)resumePoints;
    LLVMValueRef normal = evaluateNormal();
    LLVMValueRef resumed = evaluateResume();
    // Real code merges via phi; here we just return the resumed slot.
    return resumed ? resumed : normal;
}

} // namespace kotlinc_native_ref

