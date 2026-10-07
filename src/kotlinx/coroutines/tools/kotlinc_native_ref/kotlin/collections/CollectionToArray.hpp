/*
 * Copyright 2010-2023 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source libraries/stdlib/src/kotlin/collections/Collections.kt
// Transliterated from: libraries/stdlib/src/kotlin/collections/Collections.kt:514-545
#pragma once

#include "Collections.hpp"
#include "../Array.hpp"

namespace kotlin::collections {
// NOTE(port): The internal generic array functions have a concrete Any?
// instantiation for the compiler-owned erased collection boundary. The source
// collection retains its genuine star-projected covariance view and iterator.
// Other typed array instantiations are not supplied by this specialization.
// Transliterated from: libraries/stdlib/src/kotlin/collections/Collections.kt:514-526
Array<std::any> collection_to_array_common_impl(const Collection<std::any>& collection);
// Transliterated from: libraries/stdlib/src/kotlin/collections/Collections.kt:528-545
Array<std::any> collection_to_array_common_impl(const Collection<std::any>& collection,
                                              Array<std::any> array);

}  // namespace kotlin::collections
