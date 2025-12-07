#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/CloseableCoroutineDispatcher.kt
//
// TODO: actual/expect keyword not directly translatable
// TODO: AutoCloseable struct needs C++ equivalent (RAII or explicit close())

namespace kotlinx {
namespace coroutines {

// TODO: actual keyword - platform-specific implementation marker
class CloseableCoroutineDispatcher : CoroutineDispatcher {
public:
    CloseableCoroutineDispatcher() = default;

    // TODO: AutoCloseable equivalent - consider RAII or explicit close()
    virtual void close() = 0;

    virtual ~CloseableCoroutineDispatcher() = default;
};

} // namespace coroutines
} // namespace kotlinx
