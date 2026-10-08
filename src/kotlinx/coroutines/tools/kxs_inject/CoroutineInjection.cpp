// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2281-2337
// Inject Kotlin/Native address dispatch into compiler-marked coroutine bodies.
//
// Kotlin contracts: tmp/kotlin/kotlin-native/backend.native/compiler/ir/
// backend.native/src/org/jetbrains/kotlin/backend/konan/lower/
// NativeSuspendFunctionLowering.kt:253-335; CoroutinesVarSpillingLowering.kt:68-105
// and llvm/IrToBitcode.kt:2289-2348.
// Frame-field addresses and resume destinations are supplied by the frontend.
// Never infer a frame layout from an argument index or a marker's integer ID.

#include "CoroutineInjection.hpp"
#include "../kotlinc_native_ref/IrToBitcode_coroutines.hpp"
#include "../kotlinc_native_ref/CodeGenerator.hpp"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <set>
using namespace llvm;

static bool validate_marker(Function* marker, bool entry, raw_ostream& diagnostics) {
    if (!marker) return true;
    FunctionType* type = marker->getFunctionType();
    bool valid = marker->isDeclaration() && type->getReturnType()->isVoidTy() &&
                 !type->isVarArg() && type->getNumParams() == (entry ? 1U : 3U);
    if (valid) {
        valid = entry ? type->getParamType(0)->isPointerTy()
                      : type->getParamType(0)->isIntegerTy(32) &&
                        type->getParamType(1)->isPointerTy() && type->getParamType(2)->isPointerTy();
    }
    if (!valid) {
        diagnostics << "Invalid " << marker->getName() << " signature: expected "
               << (entry ? "declaration void(ptr)" : "declaration void(i32, ptr, ptr)") << "\n";
        return false;
    }
    for (User* user : marker->users()) {
        auto* call = dyn_cast<CallInst>(user);
        if (!call || call->getCalledOperand() != marker || call->hasOperandBundles()) {
            diagnostics << "Unsupported use of " << marker->getName()
                   << ": injection requires direct calls without operand bundles\n";
            return false;
        }
    }
    return true;
}

// NOTE(port): Standard C++ branches keep resume labels reachable in Clang's
// frontend. Their marker condition is erased before optimization; the branch's
// true successor supplies the actual LLVM resume block, never a runtime index.
static bool validate_resume_markers(Function* site, Function* resume, raw_ostream& diagnostics) {
    for (Function* marker : {site, resume}) {
        if (!marker) continue;
        auto* type = marker->getFunctionType();
        const bool is_site = marker == site;
        if (!marker->isDeclaration() || type->isVarArg() ||
            type->getNumParams() != (is_site ? 2U : 1U) ||
            !(is_site ? type->getReturnType()->isVoidTy() : type->getReturnType()->isIntegerTy(1)) ||
            !type->getParamType(0)->isIntegerTy(32) ||
            (is_site && !type->getParamType(1)->isPointerTy())) {
            diagnostics << "Invalid " << marker->getName() << " signature\n";
            return false;
        }
        for (User* user : marker->users()) {
            auto* call = dyn_cast<CallInst>(user);
            if (!call || call->getCalledOperand() != marker || call->hasOperandBundles()) {
                diagnostics << "Unsupported use of " << marker->getName() << "\n";
                return false;
            }
        }
    }
    return true;
}

// Clang's unoptimized frontend reloads the same frame pointer at each marker.
// Compare those address computations without treating distinct allocations or
// arbitrary calls as interchangeable values.
static bool same_field(Value* left, Value* right) {
    left = left->stripPointerCasts();
    right = right->stripPointerCasts();
    if (left == right) return true;
    if (auto* load = dyn_cast<LoadInst>(left)) {
        auto* other = dyn_cast<LoadInst>(right);
        return other && !load->isVolatile() && !other->isVolatile() &&
               !load->isAtomic() && !other->isAtomic() &&
               same_field(load->getPointerOperand(), other->getPointerOperand());
    }
    auto* gep = dyn_cast<GEPOperator>(left);
    auto* other = dyn_cast<GEPOperator>(right);
    if (!gep || !other || gep->getSourceElementType() != other->getSourceElementType() ||
        gep->getNumOperands() != other->getNumOperands()) return false;
    for (unsigned i = 0; i < gep->getNumOperands(); ++i) {
        if (!same_field(gep->getOperand(i), other->getOperand(i))) return false;
    }
    return true;
}

// NOTE(port): Marker-to-LLVM adapter for the address dispatch in
// IrToBitcode.kt:2289-2306. Expression results and resume-result generation
// still arrive from the frontend; this is not evaluateSuspensionPoint parity.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2281-2337
static bool inject_function(Function& function, Function* entry, Function* point,
                            Function* site, Function* resume,
                            raw_ostream& diagnostics, bool verbose) {
    SmallVector<CallInst*, 8> points;
    std::map<uint64_t, std::pair<CallInst*, BasicBlock*>> resume_blocks;
    CallInst* begin = nullptr;
    for (BasicBlock& block : function) {
        for (Instruction& instruction : block) {
            auto* call = dyn_cast<CallInst>(&instruction);
            if (!call) continue;
            if (entry && call->getCalledFunction() == entry) {
                if (begin) {
                    diagnostics << "Multiple coroutine entries in @" << function.getName() << "\n";
                    return false;
                }
                begin = call;
            }
            if (point && call->getCalledFunction() == point) points.push_back(call);
            if (site && call->getCalledFunction() == site) points.push_back(call);
            if (resume && call->getCalledFunction() == resume) {
                auto* id = dyn_cast<ConstantInt>(call->getArgOperand(0));
                auto* branch = call->hasOneUse() ? dyn_cast<BranchInst>(*call->user_begin()) : nullptr;
                if (!id || !branch || !branch->isConditional() || branch->getCondition() != call) {
                    diagnostics << "Resume marker needs a constant ID and direct conditional branch in @"
                                << function.getName() << "\n";
                    return false;
                }
                if (!resume_blocks.emplace(id->getZExtValue(), std::make_pair(call, branch->getSuccessor(0))).second) {
                    diagnostics << "Duplicate resume marker ID in @" << function.getName() << "\n";
                    return false;
                }
            }
        }
    }
    if (!begin && points.empty() && resume_blocks.empty()) return true;
    if (!begin) {
        diagnostics << "Missing __kxs_coroutine_begin frame field in @" << function.getName() << "\n";
        return false;
    }
    Value* label_field = begin->getArgOperand(0);
    if (isa<AllocaInst>(getUnderlyingObject(label_field)) || isa<ConstantPointerNull>(label_field)) {
        diagnostics << "Coroutine resume label must be persistent frame storage, not stack/null in @"
               << function.getName() << "\n";
        return false;
    }
    std::vector<LLVMBasicBlockRef> destinations;
    org::jetbrains::kotlin::backend::konan::llvm::SuspendableExpressionScope scope(destinations);
    std::map<CallInst*, BlockAddress*> addresses;
    std::set<uint64_t> used_resume_ids;
    for (CallInst* call : points) {
        auto* id = dyn_cast<ConstantInt>(call->getArgOperand(0));
        BlockAddress* address = nullptr;
        if (call->getCalledFunction() == site) {
            if (id) {
                auto found = resume_blocks.find(id->getZExtValue());
                if (found != resume_blocks.end() && used_resume_ids.insert(id->getZExtValue()).second)
                    address = BlockAddress::get(&function, found->second.second);
            }
            if (!address) {
                diagnostics << "Suspension site needs a unique matching resume marker in @" << function.getName() << "\n";
                return false;
            }
        } else address = dyn_cast<BlockAddress>(call->getArgOperand(2)->stripPointerCasts());
        if (!same_field(label_field, call->getArgOperand(1))) {
            diagnostics << "Suspension point uses a different frame label field in @"
                   << function.getName() << "\n";
            return false;
        }
        if (!id || !address || address->getFunction() != &function) {
            diagnostics << "Suspension point needs a constant ID and function-local blockaddress in @"
                   << function.getName() << "\n";
            return false;
        }
        if (llvm::is_contained(destinations, wrap(address->getBasicBlock()))) {
            diagnostics << "Duplicate resume destination in @" << function.getName() << "\n";
            return false;
        }
        scope.add_resume_point(wrap(address->getBasicBlock()));
        addresses.emplace(call, address);
    }
    if (used_resume_ids.size() != resume_blocks.size()) {
        diagnostics << "Resume marker has no suspension site in @" << function.getName() << "\n";
        return false;
    }

    // Keep frontend argument/result initialization before dispatch. Unlike the
    // original injector, do not move the prologue or allocate/reset a label slot.
    BasicBlock* entry_block = begin->getParent();
    BasicBlock* start = entry_block->splitBasicBlock(begin->getNextNode(), "kxs_start");
    begin->eraseFromParent();
    entry_block->getTerminator()->eraseFromParent();
    BasicBlock* dispatch = BasicBlock::Create(function.getContext(), "kxs_dispatch", &function, start);
    org::jetbrains::kotlin::backend::konan::llvm::FunctionGenerationContext generation(wrap(&function));
    generation.position_at_end(wrap(entry_block));
    Type* pointer_type = PointerType::get(function.getContext(), 0);
    LLVMValueRef saved_label = LLVMBuildLoad2(generation.builder(), wrap(pointer_type), wrap(label_field), "kxs_saved_label");
    LLVMValueRef fresh = generation.icmp_eq(saved_label, LLVMConstNull(wrap(pointer_type)), "kxs_is_first");
    generation.cond_br(fresh, wrap(start), wrap(dispatch));
    generation.position_at_end(wrap(dispatch));
    generation.indirect_br(saved_label, destinations);
    for (CallInst* call : points) {
        generation.position_before(wrap(call));
        LLVMBuildStore(generation.builder(), wrap(addresses.at(call)), wrap(label_field));
        call->eraseFromParent();
    }
    for (auto& [id, target] : resume_blocks) {
        (void)id;
        target.first->replaceAllUsesWith(ConstantInt::getFalse(function.getContext()));
        target.first->eraseFromParent();
    }
    if (verbose) diagnostics << "Injected " << destinations.size() << " suspension destinations in @"
                        << function.getName() << "\n";
    return true;
}

// NOTE(port): Clang module adapter; Kotlin evaluates expressions in its own
// code-generation context rather than discovering frontend marker declarations.
bool kotlinx::coroutines::compiler::inject_coroutines(
    Module& module, raw_ostream& diagnostics, bool verbose) {
    Function* entry = module.getFunction("__kxs_coroutine_begin");
    Function* point = module.getFunction("__kxs_suspend_point");
    Function* site = module.getFunction("__kxs_suspend_site");
    Function* resume = module.getFunction("__kxs_resume_point");
    if (!validate_marker(entry, true, diagnostics) || !validate_marker(point, false, diagnostics) ||
        !validate_resume_markers(site, resume, diagnostics)) return false;
    for (Function& function : module) {
        if (!function.isDeclaration() && !inject_function(function, entry, point, site, resume, diagnostics, verbose)) return false;
    }
    if (entry && entry->use_empty()) entry->eraseFromParent();
    if (point && point->use_empty()) point->eraseFromParent();
    if (site && site->use_empty()) site->eraseFromParent();
    if (resume && resume->use_empty()) resume->eraseFromParent();
    return true;
}
