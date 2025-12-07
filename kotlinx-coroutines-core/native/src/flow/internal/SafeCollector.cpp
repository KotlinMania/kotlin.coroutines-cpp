#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt
//
// TODO: actual keyword - platform-specific implementation
// TODO: suspend function semantics
// TODO: currentCoroutineContext function

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.coroutines.*
// import kotlinx.coroutines.flow.*
// import kotlin.coroutines.*

// TODO: actual class
template<typename T>
class SafeCollector : FlowCollector<T> {
public:
    FlowCollector<T>* collector;
    CoroutineContext collect_context;

    // Note, it is non-capturing lambda, so no extra allocation during init of SafeCollector
    int collect_context_size;

private:
    CoroutineContext* last_emission_context;

public:
    SafeCollector(FlowCollector<T>* collector, CoroutineContext collect_context)
        : collector(collector)
        , collect_context(collect_context)
        , last_emission_context(nullptr)
    {
        collect_context_size = collect_context.fold(0, [](int count, auto /* element */) {
            return count + 1;
        });
    }

    // TODO: actual override suspend function
    void emit(T value)  override {
        auto current_context = current_coroutine_context();
        current_context.ensure_active();

        if (last_emission_context != &current_context) {
            check_context(current_context);
            last_emission_context = &current_context;
        }
        collector->emit(value);
    }

    void release_intercepted() {
        // Empty implementation
    }
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
