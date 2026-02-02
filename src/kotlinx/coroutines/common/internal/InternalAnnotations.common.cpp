// port-lint: source internal/InternalAnnotations.common.kt
// Transliterated from Kotlin to C++
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: Annotations in Kotlin don't have direct C++ equivalents
// TODO: @Target, @OptionalExpectation are Kotlin-specific
// TODO: expect annotation needs platform-specific implementation or macro
// TODO: This file is primarily for metadata - may be implemented as comments or attributes

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            // Ignore JRE requirements for animal-sniffer, compileOnly dependency
            // TODO: @Target annotation - multiple targets
            // TODO: @OptionalExpectation - expect declaration
            // TODO: expect annotation class - needs platform-specific implementation or preprocessor macro

            // C++ doesn't have a direct equivalent to Kotlin annotations
            // This would typically be implemented as:
            // 1. Compiler attributes (e.g., [[deprecated]], [[nodiscard]])
            // 2. Preprocessor macros
            // 3. Comments for documentation purposes

            // Placeholder implementation as a macro:
#define IGNORE_JRE_REQUIREMENT

            // Or as a C++11 attribute (non-standard):
            // [[ignore_jre_requirement]]
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx