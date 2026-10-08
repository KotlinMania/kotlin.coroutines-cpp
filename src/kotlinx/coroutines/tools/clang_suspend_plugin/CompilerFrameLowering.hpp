// NOTE(port): Clang integration declarations for Kotlin-derived native lowering.
#pragma once
#include <string>
namespace clang { class ASTContext; class CompilerInstance; class FunctionDecl; }
namespace kotlinx::suspend {
// Build the native frame text using the compiler-derived lowering.
std::string lower_native_suspend(clang::ASTContext& context, clang::FunctionDecl* function);
bool is_lowering_parser_active();
// Build and import the retained frame before the main consumer emits the entry.
bool install_native_frame(clang::CompilerInstance& compiler, clang::FunctionDecl* function,
                          std::string lowered);
}
