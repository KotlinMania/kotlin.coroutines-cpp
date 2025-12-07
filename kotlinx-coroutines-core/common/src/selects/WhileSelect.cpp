// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/selects/WhileSelect.kt
//
// TODO: This is a mechanical syntax transliteration. The following Kotlin constructs need proper C++ implementation:
// - suspend functions (marked but not implemented as C++20 coroutines)
// - inline functions with crossinline parameters
// - Lambda types and closures
// - @ExperimentalCoroutinesApi annotation (kept as comment)

namespace kotlinx {
namespace coroutines {
namespace selects {

// import kotlinx.coroutines.*

/**
 * Loops while [select] expression returns `true`.
 *
 * The statement of the form:
 *
 * ```
 * whileSelect {
 *     /*body*/
 * }
 * ```
 *
 * is a shortcut for:
 *
 * ```
 * while(select<Boolean> {
 *    /*body*/
 * }) {}
 *
 * **Note: This is an experimental api.** It may be replaced with a higher-performance DSL for selection from loops.
 */
// @ExperimentalCoroutinesApi
template<typename BuilderFunc>
void while_select(BuilderFunc&& builder) {
    // TODO: suspend function semantics not implemented
    // TODO: inline with crossinline not directly translatable
    while (select<bool>(std::forward<BuilderFunc>(builder))) {
        /* do nothing */
    }
}

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
