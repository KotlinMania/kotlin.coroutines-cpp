// NOTE(port): Clang marker-to-LLVM module adapter; source lowering contracts
// are recorded in CoroutineInjection.cpp.
#pragma once
namespace llvm { class Module; class raw_ostream; }
namespace kotlinx::coroutines::compiler {
// Frontend frame fields and resume blocks are actual LLVM operands.
bool inject_coroutines(llvm::Module& module, llvm::raw_ostream& diagnostics,
                       bool verbose = false);
}
