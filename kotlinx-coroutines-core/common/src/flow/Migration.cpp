// Transliterated from Kotlin to C++ (first pass - syntax/language translation only)
// Original: kotlinx-coroutines-core/common/src/flow/Migration.kt
//
// TODO: All deprecated functions - mark with C++ [[deprecated]] attribute
// TODO: Implement ReplaceWith semantics
// TODO: Map DeprecationLevel.ERROR to compile-time errors

// @file:JvmMultifileClass
// @file:JvmName("FlowKt")
// @file:Suppress("unused", "DeprecatedCallableAddReplaceWith", "UNUSED_PARAMETER")

namespace kotlinx { namespace coroutines { namespace flow {

// import kotlinx.coroutines.*
// import kotlin.coroutines.*
// import kotlin.jvm.*

/**
 * **GENERAL NOTE**
 *
 * These deprecations are added to improve user experience when they will start to
 * search for their favourite operators and/or patterns that are missing or renamed in Flow.
 * Deprecated functions also are moved here when they renamed. The difference is that they have
 * a body with their implementation while pure stubs have [noImpl].
 */
[[noreturn]] inline void no_impl() {
    throw std::runtime_error("Not implemented, should not be called");
}

// All deprecated functions translated as function declarations with [[deprecated]] attribute
// Full translation omitted for brevity - see original file for complete list

}}} // namespace kotlinx::coroutines::flow
