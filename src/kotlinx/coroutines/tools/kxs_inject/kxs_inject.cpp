// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/
// org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2289-2348
// Inject Kotlin/Native address dispatch into compiler-marked coroutine bodies.
//
// Kotlin contracts: tmp/kotlin/kotlin-native/backend.native/compiler/ir/
// backend.native/src/org/jetbrains/kotlin/backend/konan/lower/
// NativeSuspendFunctionLowering.kt:253-335; CoroutinesVarSpillingLowering.kt:68-105
// and llvm/IrToBitcode.kt:2289-2348.
// Frame-field addresses and resume destinations are supplied by the frontend.
// Never infer a frame layout from an argument index or a marker's integer ID.

#include "CoroutineInjection.hpp"
#include "llvm/Bitcode/BitcodeWriter.h"
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

int main(int argc, char** argv) {
    InitLLVM init(argc, argv);
    cl::ParseCommandLineOptions(argc, argv,
        "kxs-inject - inject Kotlin/Native suspension dispatch into LLVM IR\n");
    LLVMContext context;
    SMDiagnostic error;
    std::unique_ptr<Module> module = parseIRFile(input_filename, error, context);
    if (!module) {
        error.print(argv[0], errs());
        return 1;
    }
    if (verifyModule(*module, &errs())) return 1;
    if (!kotlinx::coroutines::compiler::inject_coroutines(*module, errs(), verbose)) return 1;
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
