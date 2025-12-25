// kxs-inject: LLVM IR transformation tool for kotlinx.coroutines-cpp
//
// This tool reads LLVM IR (.ll or .bc), finds suspend points marked by
// __kxs_suspend_point() calls, and transforms them into computed goto
// dispatch (indirectbr + blockaddress) matching Kotlin/Native's pattern.
//
// See: docs/IR_SUSPEND_LOWERING_SPEC.md
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
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <vector>
#include <string>
#include <map>

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

// Suspend point info
struct SuspendPoint {
    CallInst *call;
    int id;
    BasicBlock *resumeBlock = nullptr;
};

// Find all calls to __kxs_suspend_point in a function
static std::vector<SuspendPoint> findSuspendPoints(Function &F) {
    std::vector<SuspendPoint> suspendPoints;

    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (auto *CI = dyn_cast<CallInst>(&I)) {
                if (Function *Callee = CI->getCalledFunction()) {
                    if (Callee->getName() == "__kxs_suspend_point") {
                        SuspendPoint SP;
                        SP.call = CI;
                        // Extract the suspend point ID from the argument
                        if (auto *ConstArg = dyn_cast<ConstantInt>(CI->getArgOperand(0))) {
                            SP.id = static_cast<int>(ConstArg->getSExtValue());
                        } else {
                            SP.id = static_cast<int>(suspendPoints.size());
                        }
                        suspendPoints.push_back(SP);
                    }
                }
            }
        }
    }

    return suspendPoints;
}

// Get or create the label pointer from the coroutine struct (first argument, first field)
static Value* getLabelPointer(Function &F, IRBuilder<> &Builder) {
    // The coroutine struct is passed as the first argument
    // The _label field is at offset 0
    Argument *CoroArg = F.getArg(0);

    // GEP to get the first field (index 0, 0)
    // Assuming the struct layout has _label as the first field
    Type *PtrTy = Builder.getPtrTy();

    // For opaque pointers, we just do a direct pointer access
    // The _label field is at the start of the struct
    return CoroArg;  // The pointer itself points to _label (first field)
}

// Transform a function with suspend points into computed-goto dispatch
//
// Implements the Kotlin/Native pattern:
//   1. Entry dispatch: check if _label is null, goto start or dispatch
//   2. Dispatch block: indirectbr to resume labels
//   3. At each suspend point: store blockaddress, check if suspended
//   4. Resume labels: continue execution after suspension
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

    LLVMContext &Ctx = F.getContext();
    Type *PtrTy = PointerType::get(Ctx, 0);

    // Get the entry block
    BasicBlock *EntryBlock = &F.getEntryBlock();

    // Create new blocks for dispatch logic
    BasicBlock *DispatchBlock = BasicBlock::Create(Ctx, "kxs_dispatch", &F, EntryBlock);
    BasicBlock *StartBlock = BasicBlock::Create(Ctx, "kxs_start", &F, EntryBlock);

    // Move all instructions from entry to start
    StartBlock->splice(StartBlock->begin(), EntryBlock);

    // Build the entry dispatch in the original entry block
    IRBuilder<> EntryBuilder(EntryBlock);

    // Load the saved label from the coroutine struct
    // %label_ptr = first argument (points to coroutine, _label is first field)
    Value *LabelPtr = F.getArg(0);

    // %saved_label = load ptr, ptr %label_ptr
    Value *SavedLabel = EntryBuilder.CreateLoad(PtrTy, LabelPtr, "kxs_saved_label");

    // %is_first = icmp eq ptr %saved_label, null
    Value *NullPtr = ConstantPointerNull::get(cast<PointerType>(PtrTy));
    Value *IsFirst = EntryBuilder.CreateICmpEQ(SavedLabel, NullPtr, "kxs_is_first");

    // br i1 %is_first, label %start, label %dispatch
    EntryBuilder.CreateCondBr(IsFirst, StartBlock, DispatchBlock);

    // Create resume blocks for each suspend point by splitting
    std::vector<BasicBlock*> resumeBlocks;
    for (size_t i = 0; i < suspendPoints.size(); i++) {
        SuspendPoint &SP = suspendPoints[i];
        CallInst *CI = SP.call;
        BasicBlock *BB = CI->getParent();

        // Split the block after the suspend point call
        // The new block becomes the resume point
        BasicBlock *ResumeBlock = BB->splitBasicBlock(
            CI->getNextNode(),
            "kxs_resume_" + std::to_string(SP.id)
        );
        SP.resumeBlock = ResumeBlock;
        resumeBlocks.push_back(ResumeBlock);

        if (Verbose) {
            errs() << "  Created resume block: " << ResumeBlock->getName()
                   << " for suspend point " << SP.id << "\n";
        }
    }

    // Build the dispatch block with indirectbr
    IRBuilder<> DispatchBuilder(DispatchBlock);
    IndirectBrInst *IndBr = DispatchBuilder.CreateIndirectBr(SavedLabel, resumeBlocks.size());
    for (BasicBlock *BB : resumeBlocks) {
        IndBr->addDestination(BB);
    }

    // Insert blockaddress stores before each suspend point
    for (size_t i = 0; i < suspendPoints.size(); i++) {
        SuspendPoint &SP = suspendPoints[i];
        CallInst *CI = SP.call;

        // Insert before the suspend point call
        IRBuilder<> Builder(CI);

        // Get blockaddress for the resume block
        BlockAddress *ResumeAddr = BlockAddress::get(&F, SP.resumeBlock);

        // Store the blockaddress to the label pointer
        Builder.CreateStore(ResumeAddr, LabelPtr);

        if (Verbose) {
            errs() << "  Inserted blockaddress store for resume_" << SP.id << "\n";
        }
    }

    // Remove the marker calls (they're just placeholders)
    for (SuspendPoint &SP : suspendPoints) {
        // Get the terminator that was created by splitBasicBlock
        BasicBlock *BB = SP.call->getParent();

        // Remove the call
        SP.call->eraseFromParent();
    }

    if (Verbose) {
        errs() << "  Transformation complete\n";
    }

    return true;
}

int main(int argc, char **argv) {
    InitLLVM X(argc, argv);

    cl::ParseCommandLineOptions(argc, argv,
        "kxs-inject - LLVM IR coroutine transformation tool\n"
        "\n"
        "Transforms __kxs_suspend_point() markers into Kotlin/Native-style\n"
        "computed goto dispatch (indirectbr + blockaddress).\n"
        "\n"
        "See: docs/IR_SUSPEND_LOWERING_SPEC.md\n");

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
