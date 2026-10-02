// LLVM-aware cleanup of no-op markers in already lowered coroutine functions.
// The frame layout, spill stores/loads, dispatch and Result paths are preserved.
//
// Kotlin contracts: tmp/kotlin/kotlin-native/backend.native/compiler/ir/
// backend.native/src/org/jetbrains/kotlin/backend/konan/lower/
// NativeSuspendFunctionLowering.kt:253-335; CoroutinesVarSpillingLowering.kt:68-105
// and llvm/IrToBitcode.kt:2289-2348.
// NOTE(port): Cleanup is build infrastructure, not a coroutine lowering pass.

#include "llvm/ADT/SmallVector.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>
#include <string>

using namespace llvm;

static cl::opt<std::string> input_filename(cl::Positional,
    cl::desc("<input .ll or .bc file>"), cl::Required);
static cl::opt<std::string> output_filename("o",
    cl::desc("Output filename"), cl::value_desc("filename"), cl::init("-"));
static cl::opt<bool> output_bitcode("bc",
    cl::desc("Output as bitcode instead of text IR"), cl::init(false));
static cl::opt<bool> verbose("v", cl::desc("Verbose output"), cl::init(false));

// Remove direct calls only. Invokes keep their unwind edges and the declaration
// remains whenever any use (including an alias or function pointer) survives.
static bool strip_markers(Module& module) {
    Function* marker = module.getFunction("__kxs_suspend_point");
    if (!marker) return true;
    FunctionType* type = marker->getFunctionType();
    if (!type->getReturnType()->isVoidTy() || type->isVarArg() ||
        type->getNumParams() != 1 || !type->getParamType(0)->isIntegerTy(32)) {
        errs() << "Invalid __kxs_suspend_point signature: expected void(i32)\n";
        return false;
    }
    unsigned removed = 0;
    for (Function& function : module) {
        SmallVector<CallInst*, 8> calls;
        for (BasicBlock& block : function) {
            for (Instruction& instruction : block) {
                auto* call = dyn_cast<CallInst>(&instruction);
                if (call && call->getCalledOperand()->stripPointerCasts() == marker)
                    calls.push_back(call);
            }
        }
        for (CallInst* call : calls) call->eraseFromParent();
        removed += calls.size();
        if (verbose && !calls.empty())
            errs() << "Removed " << calls.size() << " marker(s) in @"
                   << function.getName() << "; preserved coroutine IR\n";
    }
    if (marker->isDeclaration() && marker->use_empty()) marker->eraseFromParent();
    if (verbose) errs() << "Removed " << removed << " marker call(s)\n";
    return true;
}

int main(int argc, char** argv) {
    InitLLVM init(argc, argv);
    cl::ParseCommandLineOptions(argc, argv,
        "kxs-inject - verify IR and remove no-op suspend markers\n");
    LLVMContext context;
    SMDiagnostic error;
    std::unique_ptr<Module> module = parseIRFile(input_filename, error, context);
    if (!module) {
        error.print(argv[0], errs());
        return 1;
    }
    if (verifyModule(*module, &errs())) return 1;
    if (!strip_markers(*module)) return 1;
    if (verifyModule(*module, &errs())) return 1;

    std::error_code ec;
    ToolOutputFile output(output_filename, ec, sys::fs::OF_None);
    if (ec) {
        errs() << "Error opening output: " << ec.message() << "\n";
        return 1;
    }
    if (output_bitcode) WriteBitcodeToFile(*module, output.os());
    else module->print(output.os(), nullptr);
    output.keep();
    return 0;
}
