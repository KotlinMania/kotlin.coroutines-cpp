// port-lint: source internal/InternalAnnotations.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/InternalAnnotations.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   @OptionalExpectation
 *   @Target(AnnotationTarget.CLASS, AnnotationTarget.PROPERTY, ...)
 *   internal expect annotation class IgnoreJreRequirement()
 *
 * The annotation exists only to silence the JVM animal-sniffer compileOnly dependency
 * during Kotlin compilation. It carries no runtime behavior. In C++ the equivalent role
 * is filled by a tag macro that expands to nothing — call sites can leave it in place as
 * documentation, and there is no analogue of @OptionalExpectation since the annotation
 * has no per-platform actuals.
 */

#ifndef KOTLINX_COROUTINES_IGNORE_JRE_REQUIREMENT
#define KOTLINX_COROUTINES_IGNORE_JRE_REQUIREMENT
#endif
