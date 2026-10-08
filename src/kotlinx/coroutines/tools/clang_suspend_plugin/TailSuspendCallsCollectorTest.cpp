#include "TailSuspendCallsCollector.hpp"
#include "clang/AST/AST.h"
#include "clang/Tooling/Tooling.h"
#include <cassert>
#include <string>
#include <vector>

int main() {
    struct Case { std::string body; unsigned tail_count; bool has_non_tail; bool unit = false; };
    const std::vector<Case> cases = {
        {"return source(nullptr);", 1, false},
        {"return (ordinary(nullptr), source(nullptr));", 1, false},
        {"return (source(nullptr), source(nullptr));", 1, true},
        {"return (ordinary(nullptr), (ordinary(nullptr), source(nullptr)));", 1, false},
        {"return (source(nullptr), ordinary(nullptr));", 0, true},
        {"return ordinary((ordinary(nullptr), source(nullptr)));", 0, true},
        {"return (Marker{}, source(nullptr));", 0, true},
        {"return choose ? (ordinary(nullptr), source(nullptr)) : source(nullptr);", 2, false},
        {"try { return (ordinary(nullptr), source(nullptr)); } catch (...) { return nullptr; }", 0, true},
        {"return choose ? source(nullptr) : source(nullptr);", 2, false},
        {"return source(nullptr) ? source(nullptr) : source(nullptr);", 2, true},
        {"return ordinary(source(nullptr));", 0, true},
        {"return source(source(nullptr));", 1, true},
        {"return kotlinx::coroutines::dsl::suspend(source(nullptr));", 1, false},
        {"return static_cast<void*>(source(nullptr));", 0, true},
        {"try { return source(nullptr); } catch (...) { return source(nullptr); }", 1, true},
        {"auto block = [] { return source(nullptr); }; return nullptr;", 0, false},
        {"auto block = [value = source(nullptr)] { return value; }; return nullptr;", 0, true},
        {"source(nullptr); return nullptr;", 1, false, true},
        {"source(nullptr); return (nullptr);", 1, false, true},
        {"(ordinary(nullptr), source(nullptr)); return ((nullptr));", 1, false, true},
        {"source(nullptr); return nullptr;", 0, true},
        {"if (choose) return source(nullptr); return source(nullptr);", 2, false},
        {"if (source(nullptr)) return source(nullptr); return nullptr;", 1, true},
        {"if (void* value = source(nullptr); choose) return value; return nullptr;", 0, true},
        {"if (choose) { source(nullptr); return nullptr; } return nullptr;", 1, false, true}
    };
    for (const auto& test : cases) {
        auto ast = clang::tooling::buildASTFromCodeWithArgs(
            "[[clang::annotate(\"suspend\")]] void* source(void*); void* ordinary(void*);"
            "struct Marker {}; void* operator,(Marker, void*);"
            "namespace kotlinx::coroutines::dsl { template<class T> T suspend(T&&); }"
            "[[clang::annotate(\"suspend\")]] " +
            std::string(test.unit ? "[[clang::annotate(\"kotlin.ir.UnitReturn\")]] " : "") +
            "void* probe(bool choose) { " + test.body + " }", {"-std=c++20"});
        assert(ast && !ast->getDiagnostics().hasErrorOccurred());
        const clang::FunctionDecl* function = nullptr;
        for (const auto* declaration : ast->getASTContext().getTranslationUnitDecl()->decls())
            if (const auto* candidate = llvm::dyn_cast<clang::FunctionDecl>(declaration);
                candidate && candidate->getIdentifier() && candidate->getName() == "probe") function = candidate;
        assert(function);
        const auto result = org::jetbrains::kotlin::backend::common::collect_tail_suspend_calls(function);
        assert(result.call_sites.size() == test.tail_count);
        assert(result.has_not_tail_suspend_calls == test.has_non_tail);
    }
}
