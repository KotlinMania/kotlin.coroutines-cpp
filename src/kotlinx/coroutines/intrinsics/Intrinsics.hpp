/*
 * Copyright 2010-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 *
 * C++ transliteration of kotlin/coroutines/intrinsics/Intrinsics.kt
 */

#pragma once

#include <cstdint>

namespace kotlinx {
namespace coroutines {
namespace intrinsics {

/**
 * Singleton enum for coroutine state markers.
 * Using enum here ensures:
 *  1. It makes SafeContinuation serializable
 *  2. It improves debugging experience with clear toString() values
 *
 * Transliterated from: CoroutineSingletons enum in Intrinsics.kt
 */
enum class CoroutineSingletons {
    COROUTINE_SUSPENDED,
    UNDECIDED,
    RESUMED
};

/**
 * This value is used as a return value of suspendCoroutineUninterceptedOrReturn
 * block argument to state that the execution was suspended and will not return
 * any result immediately.
 *
 * In C++, we represent this as a void* pointer to a static marker.
 * Any suspend function that returns this marker indicates suspension.
 *
 * Transliterated from: val COROUTINE_SUSPENDED in Intrinsics.kt
 */
inline void* get_COROUTINE_SUSPENDED() {
    // Static marker - the address itself is the unique identifier
    static auto marker = CoroutineSingletons::COROUTINE_SUSPENDED;
    return &marker;
}

/**
 * Macro for convenience - matches Kotlin's COROUTINE_SUSPENDED constant.
 */
#define COROUTINE_SUSPENDED (::kotlinx::coroutines::intrinsics::get_COROUTINE_SUSPENDED())

/**
 * Check if a result value indicates suspension.
 *
 * In Kotlin: outcome === COROUTINE_SUSPENDED
 * In C++: pointer comparison to the marker
 */
inline bool is_coroutine_suspended(void* result) {
    return result == get_COROUTINE_SUSPENDED();
}

} // namespace intrinsics
} // namespace coroutines
} // namespace kotlinx
