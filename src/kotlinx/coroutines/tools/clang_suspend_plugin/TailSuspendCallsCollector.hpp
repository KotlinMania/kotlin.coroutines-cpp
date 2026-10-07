// port-lint: source compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:17-123
#pragma once
#include <set>
namespace clang { class FunctionDecl; class CallExpr; }
namespace org::jetbrains::kotlin::backend::common {
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:17
struct TailSuspendCalls {
    std::set<const clang::CallExpr*> call_sites;
    bool has_not_tail_suspend_calls;
};
// Collect last expressions that return another suspend function's result.
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/TailSuspendCallsCollector.kt:19-123
TailSuspendCalls collect_tail_suspend_calls(const clang::FunctionDecl* function);
}
