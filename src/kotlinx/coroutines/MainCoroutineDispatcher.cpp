/**
 * Transliterated from: kotlinx-coroutines-core/common/src/MainCoroutineDispatcher.kt
 */

#include "kotlinx/coroutines/MainCoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

namespace kotlinx::coroutines {

/**
 * Upstream:
 *   override fun toString(): String = toStringInternalImpl() ?: "$classSimpleName@$hexAddress"
 */
std::string MainCoroutineDispatcher::to_string() const {
    auto internal = to_string_internal_impl();
    if (internal.has_value()) return *internal;
    std::ostringstream os;
    os << "MainCoroutineDispatcher@" << std::hex << reinterpret_cast<std::uintptr_t>(this);
    return os.str();
}

/**
 * Upstream:
 *   override fun limitedParallelism(parallelism: Int, name: String?): CoroutineDispatcher {
 *       parallelism.checkParallelism()
 *       return namedOrThis(name)
 *   }
 *
 * `checkParallelism()` requires `parallelism >= 1`; `namedOrThis(name)` returns `this` when
 * `name` is null/empty, and a `NamedDispatcher` wrapper otherwise. Until `NamedDispatcher` is
 * fully wired here, the short-circuit branch returns `this` for any name.
 */
std::shared_ptr<CoroutineDispatcher> MainCoroutineDispatcher::limited_parallelism(
    int parallelism, const std::string& /*name*/) {
    if (parallelism < 1) {
        throw std::invalid_argument("Expected positive parallelism level, but got " +
                                    std::to_string(parallelism));
    }
    return std::dynamic_pointer_cast<CoroutineDispatcher>(this->shared_from_this());
}

/**
 * Upstream:
 *   @InternalCoroutinesApi
 *   protected fun toStringInternalImpl(): String? {
 *       val main = Dispatchers.Main
 *       if (this === main) return "Dispatchers.Main"
 *       val immediate = try { main.immediate } catch (e: UnsupportedOperationException) { null }
 *       if (this === immediate) return "Dispatchers.Main.immediate"
 *       return null
 *   }
 */
std::optional<std::string> MainCoroutineDispatcher::to_string_internal_impl() const {
    auto& main = Dispatchers::Main();
    if (this == &main) return std::string("Dispatchers.Main");
    try {
        auto& immediate_dispatcher = const_cast<MainCoroutineDispatcher&>(main).immediate();
        if (this == &immediate_dispatcher) return std::string("Dispatchers.Main.immediate");
    } catch (const std::logic_error&) {
        // `immediate` is not supported on every platform — swallow and fall through.
    }
    return std::nullopt;
}

} // namespace kotlinx::coroutines
