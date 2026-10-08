// port-lint: source core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt:73-81
#include "StandardNames.hpp"

namespace org::jetbrains::kotlin::builtins {
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt:73-74
const name::Name& StandardNames::built_ins_package_name() {
    static const name::Name BUILT_INS_PACKAGE_NAME = name::Name::identifier(u"kotlin");
    return BUILT_INS_PACKAGE_NAME;
}
// Transliterated from: core/compiler.common/src/org/jetbrains/kotlin/builtins/StandardNames.kt:80-81
const name::FqName& StandardNames::built_ins_package_fq_name() {
    static const name::FqName BUILT_INS_PACKAGE_FQ_NAME =
        name::FqName::top_level(built_ins_package_name());
    return BUILT_INS_PACKAGE_FQ_NAME;
}
}
