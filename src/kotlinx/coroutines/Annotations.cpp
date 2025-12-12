// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/Annotations.kt
//
// TODO: Annotations are not directly translatable to C++
// TODO: @MustBeDocumented, @Retention, @RequiresOptIn, @Target have no C++ equivalents
// TODO: Consider using preprocessor macros or attributes for similar behavior
// TODO: Annotation classes map to empty marker types or documentation comments

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.flow.* - removed, use fully qualified names or add includes

        /**
 * Marks declarations in the coroutines that are **delicate** &mdash;
 * they have limited use-case and shall be used with care in general code.
 * Any use of a delicate declaration has to be carefully reviewed to make sure it is
 * properly used and does not create problems like memory and resource leaks.
 * Carefully read documentation of any declaration marked as `DelicateCoroutinesApi`.
 */
        // TODO: @MustBeDocumented - no C++ equivalent
        // TODO: @Retention(value = AnnotationRetention.BINARY) - no C++ equivalent
        // TODO: @RequiresOptIn - no C++ equivalent, consider using [[deprecated]] or custom attributes
        struct DelicateCoroutinesApi {
            // Annotation marker type
        };

        /**
 * Marks declarations that are still **experimental** in coroutines API, which means that the design of the
 * corresponding declarations has open issues which may (or may not) lead to their changes in the future.
 * Roughly speaking, there is a chance that those declarations will be deprecated in the near future or
 * the semantics of their behavior may change in some way that may break some code.
 */
        // TODO: @MustBeDocumented - no C++ equivalent
        // TODO: @Retention(value = AnnotationRetention.BINARY) - no C++ equivalent
        // TODO: @Target(...) - no C++ equivalent
        // TODO: @RequiresOptIn(level = RequiresOptIn.Level.WARNING) - no C++ equivalent
        struct ExperimentalCoroutinesApi {
            // Annotation marker type
        };

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
        // TODO: @MustBeDocumented - no C++ equivalent
        // TODO: @Retention(value = AnnotationRetention.BINARY) - no C++ equivalent
        // TODO: @RequiresOptIn - no C++ equivalent
        // TODO: @Target(AnnotationTarget.CLASS, AnnotationTarget.FUNCTION, AnnotationTarget.TYPEALIAS, AnnotationTarget.PROPERTY)
        struct FlowPreview {
            // Annotation marker type
        };

        /**
 * Marks declarations that are **obsolete** in coroutines API, which means that the design of the corresponding
 * declarations has serious known flaws and they will be redesigned in the future.
 * Roughly speaking, these declarations will be deprecated in the future but there is no replacement for them yet,
 * so they cannot be deprecated right away.
 */
        // TODO: @MustBeDocumented - no C++ equivalent
        // TODO: @Retention(value = AnnotationRetention.BINARY) - no C++ equivalent
        // TODO: @RequiresOptIn(level = RequiresOptIn.Level.WARNING) - no C++ equivalent
        struct ObsoleteCoroutinesApi {
            // Annotation marker type
        };

        /**
 * Marks declarations that are **internal** in coroutines API, which means that should not be used outside of
 * `kotlinx.coroutines`, because their signatures and semantics will change between future releases without any
 * warnings and without providing any migration aids.
 */
        // TODO: @MustBeDocumented - no C++ equivalent
        // TODO: @Retention(value = AnnotationRetention.BINARY) - no C++ equivalent
        // TODO: @Target(AnnotationTarget.CLASS, AnnotationTarget.FUNCTION, AnnotationTarget.TYPEALIAS, AnnotationTarget.PROPERTY)
        // TODO: @RequiresOptIn - no C++ equivalent
        struct InternalCoroutinesApi {
            // Annotation marker type
        };

        /**
 * Marks declarations that cannot be safely inherited from.
 */
        // TODO: @Target(AnnotationTarget.CLASS) - no C++ equivalent
        // TODO: @RequiresOptIn - no C++ equivalent
        struct ExperimentalForInheritanceCoroutinesApi {
            // Annotation marker type
        };

        /**
 * Marks declarations that cannot be safely inherited from.
 */
        // TODO: @Target(AnnotationTarget.CLASS) - no C++ equivalent
        // TODO: @RequiresOptIn - no C++ equivalent
        struct InternalForInheritanceCoroutinesApi {
            // Annotation marker type
        };
    } // namespace coroutines
} // namespace kotlinx