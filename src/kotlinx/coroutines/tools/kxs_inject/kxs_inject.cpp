// kxs-inject: LLVM IR transformation tool for kotlinx.coroutines-cpp
//
// This tool reads LLVM IR (.ll or .bc), finds suspend points marked by
// __kxs_suspend_point() calls, and transforms them into computed goto
// dispatch (indirectbr + blockaddress) matching Kotlin/Native's pattern.
//
// Pipeline: .cpp -> clang -emit-llvm -> .ll -> kxs-inject -> .ll -> clang -> .o
//
// Usage: kxs-inject input.ll -o output.ll

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>
#include <string>

using namespace llvm;

// Command line options
static cl::opt<std::string> InputFilename(cl::Positional,
    cl::desc("<input .ll or .bc file>"),
    cl::Required);

static cl::opt<std::string> OutputFilename("o",
    cl::desc("Output filename"),
    cl::value_desc("filename"),
    cl::init("-"));

static cl::opt<bool> OutputBitcode("bc",
    cl::desc("Output as bitcode (.bc) instead of text IR (.ll)"),
    cl::init(false));

static cl::opt<bool> Verbose("v",
    cl::desc("Verbose output"),
    cl::init(false));

// Find all calls to __kxs_suspend_point in a function
static std::vector<CallInst*> findSuspendPoints(Function &F) {
    std::vector<CallInst*> suspendPoints;

    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (auto *CI = dyn_cast<CallInst>(&I)) {
                if (Function *Callee = CI->getCalledFunction()) {
                    if (Callee->getName() == "__kxs_suspend_point") {
                        suspendPoints.push_back(CI);
                    }
                }
            }
        }
    }

    return suspendPoints;
}

// Transform a function with suspend points into computed-goto dispatch
//
// Before:
//   entry:
//     ... code ...
//     call void @__kxs_suspend_point(i32 1)
//     ... more code ...
//
// After:
//   entry:
//     %label_ptr = getelementptr %Coroutine, ptr %coro, i32 0, i32 0  ; label field
//     %saved_label = load ptr, ptr %label_ptr
//     %is_null = icmp eq ptr %saved_label, null
//     br i1 %is_null, label %start, label %dispatch
//
//   dispatch:
//     indirectbr ptr %saved_label, [label %resume0, label %resume1, ...]
//
//   start:
//     ... code before suspend ...
//     store ptr blockaddress(@func, %resume0), ptr %label_ptr
//     ; spill live variables
//     ; call actual suspend function
//     ; if suspended, return COROUTINE_SUSPENDED
//
//   resume0:
//     ; restore live variables
//     ... code after suspend ...
//
static bool transformFunction(Function &F) {
    auto suspendPoints = findSuspendPoints(F);

    if (suspendPoints.empty()) {
        return false;
    }

    if (Verbose) {
        errs() << "Transforming function: " << F.getName()
               << " with " << suspendPoints.size() << " suspend points\n";
    }

    // For now, just demonstrate that we found the suspend points
    // Full transformation will:
    // 1. Create dispatch block with indirectbr
    // 2. Split at each suspend point to create resume labels
    // 3. Generate blockaddress constants
    // 4. Add spill/restore code based on liveness analysis

    LLVMContext &Ctx = F.getContext();
    IRBuilder<> Builder(Ctx);

    // Create resume blocks for each suspend point
    std::vector<BasicBlock*> resumeBlocks;
    for (size_t i = 0; i < suspendPoints.size(); i++) {
        CallInst *SP = suspendPoints[i];
        BasicBlock *BB = SP->getParent();

        // Split the block after the suspend point call
        BasicBlock *ResumeBlock = BB->splitBasicBlock(
            SP->getNextNode(),
            "kxs_resume" + std::to_string(i)
        );
        resumeBlocks.push_back(ResumeBlock);

        if (Verbose) {
            errs() << "  Created resume block: " << ResumeBlock->getName() << "\n";
        }
    }

    // Remove the marker calls (they're placeholders)
    for (CallInst *SP : suspendPoints) {
        SP->eraseFromParent();
    }

    // TODO: Insert dispatch block at function entry
    // TODO: Generate blockaddress constants for each resume block
    // TODO: Create indirectbr instruction
    // TODO: Add label store before each original suspend point

    return true;
}

int main(int argc, char **argv) {
    InitLLVM X(argc, argv);

    cl::ParseCommandLineOptions(argc, argv,
        "kxs-inject - LLVM IR coroutine transformation tool\n");

    // Parse input file
    LLVMContext Context;
    SMDiagnostic Err;

    std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
    if (!M) {
        Err.print(argv[0], errs());
        return 1;
    }

    if (Verbose) {
        errs() << "Loaded module: " << M->getName() << "\n";
    }

    // Transform functions with suspend points
    bool Changed = false;
    for (Function &F : *M) {
        if (!F.isDeclaration()) {
            Changed |= transformFunction(F);
        }
    }

    if (!Changed && Verbose) {
        errs() << "No suspend points found, module unchanged\n";
    }

    // Write output
    std::error_code EC;
    std::unique_ptr<ToolOutputFile> Out(
        new ToolOutputFile(OutputFilename, EC, sys::fs::OF_None));

    if (EC) {
        errs() << "Error opening output file: " << EC.message() << "\n";
        return 1;
    }

    if (OutputBitcode) {
        WriteBitcodeToFile(*M, Out->os());
    } else {
        M->print(Out->os(), nullptr);
    }

    Out->keep();

    if (Verbose) {
        errs() << "Wrote output to: " << OutputFilename << "\n";
    }

    return 0;
}
