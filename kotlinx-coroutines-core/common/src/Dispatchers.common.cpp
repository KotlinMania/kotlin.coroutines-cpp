/**
 * @file Dispatchers.common.cpp
 * @brief Common dispatcher utilities (platform-independent)
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Dispatchers.common.kt
 *
 * NOTE: The actual Dispatchers::get_* implementations are in platform-specific files:
 * - native/src/Dispatchers.cpp for native platforms
 * - nativeDarwin/src/Dispatchers.cpp for Darwin-specific overrides
 *
 * This file contains only common utilities that are platform-independent.
 */

#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"

namespace kotlinx {
namespace coroutines {

// Platform-independent dispatcher utilities can go here.
// The actual Dispatchers::get_* methods are implemented in platform-specific files.

} // namespace coroutines
} // namespace kotlinx
