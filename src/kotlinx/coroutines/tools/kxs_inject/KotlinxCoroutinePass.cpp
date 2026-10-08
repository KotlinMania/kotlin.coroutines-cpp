// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/
// org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2289-2348
// Required Kotlin/Native address lowering inside Clang's own LLVM pipeline.
#include "CoroutineInjection.hpp"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#if __has_include("llvm/Plugins/PassPlugin.h")
#include "llvm/Plugins/PassPlugin.h"
#else
#include "llvm/Passes/PassPlugin.h"
#endif
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/
// org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2289-2348
class KotlinxCoroutinePass : public PassInfoMixin<KotlinxCoroutinePass> {
public:
    PreservedAnalyses run(Module& module, ModuleAnalysisManager&) {
        if (!module.getFunction("__kxs_coroutine_begin") &&
            !module.getFunction("__kxs_suspend_point") &&
            !module.getFunction("__kxs_suspend_site") &&
            !module.getFunction("__kxs_resume_point")) return PreservedAnalyses::all();
        std::string message;
        raw_string_ostream diagnostics(message);
        if (verifyModule(module, &diagnostics) ||
            !kotlinx::coroutines::compiler::inject_coroutines(module, diagnostics) ||
            verifyModule(module, &diagnostics)) {
            module.getContext().emitError("kotlinx coroutine lowering: " + message);
        }
        return PreservedAnalyses::none();
    }

    static bool isRequired() { return true; }
};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "KotlinxCoroutinePass", LLVM_VERSION_STRING,
        [](PassBuilder& builder) {
            builder.registerPipelineStartEPCallback(
                [](ModulePassManager& pipeline, OptimizationLevel) {
                    pipeline.addPass(KotlinxCoroutinePass());
                });
            builder.registerPipelineParsingCallback(
                [](StringRef name, ModulePassManager& pipeline,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (name != "kotlinx-coroutines") return false;
                    pipeline.addPass(KotlinxCoroutinePass());
                    return true;
                });
        }};
}
