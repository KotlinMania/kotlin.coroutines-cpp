/**
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/WhileSelect.kt
 *
 * The upstream Kotlin file declares a single `inline suspend fun whileSelect` and no other
 * runtime state. The C++ port keeps the entire body inline in the matching header; this
 * source file exists only so the build system has a translation unit to compile when the
 * header is included via the inventory pass.
 */
#include "kotlinx/coroutines/selects/WhileSelect.hpp"
