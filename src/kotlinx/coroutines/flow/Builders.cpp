// port-lint: source kotlinx-coroutines-core/common/src/flow/Builders.kt
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"

namespace kotlinx::coroutines::flow {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:180-184
// NOTE(port): Kotlin's inclusive range iterator stops at last before incrementing.
[[clang::annotate("suspend")]]
void* collect_range(int first, int last, FlowCollector<int>* collector,
                    std::shared_ptr<Continuation<void*>> completion) {
    if (first <= last) {
        int value = first;
        while (true) {
            dsl::suspend(collector->emit(value, completion.get()));
            if (value == last) break;
            ++value;
        }
    }
    return nullptr;
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:189-193
// NOTE(port): Preserve the existing C++ long binding; stop before overflow.
[[clang::annotate("suspend")]]
void* collect_range(long first, long last, FlowCollector<long>* collector,
                    std::shared_ptr<Continuation<void*>> completion) {
    if (first <= last) {
        long value = first;
        while (true) {
            dsl::suspend(collector->emit(value, completion.get()));
            if (value == last) break;
            ++value;
        }
    }
    return nullptr;
}

} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:180-184
std::shared_ptr<Flow<int>> as_flow_range(int first, int last) {
    return internal::unsafe_flow<int>([first, last](FlowCollector<int>* collector,
                                                   Continuation<void*>* completion) -> void* {
        return collect_range(first, last, collector,
            kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:189-193
std::shared_ptr<Flow<long>> as_flow_range(long first, long last) {
    return internal::unsafe_flow<long>([first, last](FlowCollector<long>* collector,
                                                    Continuation<void*>* completion) -> void* {
        return collect_range(first, last, collector,
            kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

} // namespace kotlinx::coroutines::flow
