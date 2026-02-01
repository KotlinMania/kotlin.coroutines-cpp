#pragma once
// port-lint: source Annotations.kt
/**
 * @file Annotations.hpp
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Annotations.kt
 *
 * Kotlin annotations have no direct C++ equivalent, so they are represented as
 * empty marker types for API-surface parity and documentation purposes.
 */

#include "kotlinx/coroutines/flow/Flow.hpp"

namespace kotlinx::coroutines {

/**
 * Marks declarations in the coroutines that are **delicate** &mdash;
 * they have limited use-case and shall be used with care in general code.
 * Any use of a delicate declaration has to be carefully reviewed to make sure it is
 * properly used and does not create problems like memory and resource leaks.
 * Carefully read documentation of any declaration marked as `DelicateCoroutinesApi`.
 */
struct DelicateCoroutinesApi {};

/**
 * Marks declarations that are still **experimental** in coroutines API, which means that the design of the
 * corresponding declarations has open issues which may (or may not) lead to their changes in the future.
 * Roughly speaking, there is a chance that those declarations will be deprecated in the near future or
 * the semantics of their behavior may change in some way that may break some code.
 */
struct ExperimentalCoroutinesApi {};

/**
 * Marks [Flow]-related API as a feature preview.
 *
 * Flow preview has **no** backward compatibility guarantees, including both binary and source compatibility.
 * Its API and semantics can and will be changed in next releases.
 *
 * Feature preview can be used to evaluate its real-world strengths and weaknesses, gather and provide feedback.
 * According to the feedback, [Flow] will be refined on its road to stabilization and promotion to a stable API.
 *
 * The best way to speed up preview feature promotion is providing the feedback on the feature.
 */
struct FlowPreview {};

/**
 * Marks declarations that are **obsolete** in coroutines API, which means that the design of the corresponding
 * declarations has serious known flaws and they will be redesigned in the future.
 * Roughly speaking, these declarations will be deprecated in the future but there is no replacement for them yet,
 * so they cannot be deprecated right away.
 */
struct ObsoleteCoroutinesApi {};

/**
 * Marks declarations that are **internal** in coroutines API, which means that should not be used outside of
 * `kotlinx.coroutines`, because their signatures and semantics will change between future releases without any
 * warnings and without providing any migration aids.
 */
struct InternalCoroutinesApi {};

/**
 * Marks declarations that cannot be safely inherited from.
 */
struct ExperimentalForInheritanceCoroutinesApi {};

/**
 * Marks declarations that cannot be safely inherited from.
 */
struct InternalForInheritanceCoroutinesApi {};

} // namespace kotlinx::coroutines

