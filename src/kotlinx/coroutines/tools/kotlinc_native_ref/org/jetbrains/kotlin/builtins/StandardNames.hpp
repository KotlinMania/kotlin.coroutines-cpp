// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt:73-81
#pragma once
#include "../name/FqName.hpp"

namespace org::jetbrains::kotlin::builtins {
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt:18-18,73-81
class StandardNames final {
public:
    // NOTE(port): Companion properties use local-static getters to avoid
    // dependencies on C++ translation-unit initialization order.
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt:73-74
    static const name::Name& built_ins_package_name();
    // Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt:80-81
    static const name::FqName& built_ins_package_fq_name();
};
}
