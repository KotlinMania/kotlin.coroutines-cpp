// port-lint: source kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/DebugProbes.kt
/** Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/DebugProbes.kt */
#pragma once
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlin::coroutines::native::internal {
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/DebugProbes.kt:44-46
inline std::shared_ptr<kotlin::coroutines::Continuation<void*>> probe_coroutine_created(
    std::shared_ptr<kotlin::coroutines::Continuation<void*>> completion) { return completion; }
// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/DebugProbes.kt:58-60
void probe_coroutine_resumed(kotlin::coroutines::Continuation<void*>* frame);
}

