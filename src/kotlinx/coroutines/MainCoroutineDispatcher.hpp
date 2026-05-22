#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/MainCoroutineDispatcher.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 */

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"

#include <memory>
#include <optional>
#include <string>

namespace kotlinx::coroutines {

/**
 * Base class for special [CoroutineDispatcher] which is confined to application "Main" or "UI"
 * thread and used for any UI-based activities. Instance of `MainDispatcher` can be obtained
 * by [Dispatchers.Main].
 *
 * Platform may or may not provide instance of `MainDispatcher`, see documentation to
 * [Dispatchers.Main].
 *
 * Upstream:
 *   public abstract class MainCoroutineDispatcher : CoroutineDispatcher() { ... }
 */
class MainCoroutineDispatcher : public CoroutineDispatcher {
public:
    ~MainCoroutineDispatcher() override = default;

    /**
     * Returns dispatcher that executes coroutines immediately when it is already in the right
     * context (e.g. current looper is the same as this handler's looper) without an additional
     * re-dispatch.
     *
     * Method may throw [UnsupportedOperationException] if immediate dispatching is not supported
     * by current dispatcher; please refer to specific dispatcher documentation.
     *
     * Upstream: public abstract val immediate: MainCoroutineDispatcher
     */
    virtual MainCoroutineDispatcher& immediate() = 0;

    /**
     * Returns a name of this main dispatcher for debugging purposes. This implementation returns
     * `Dispatchers.Main` or `Dispatchers.Main.immediate` if it is the same as the corresponding
     * reference in [Dispatchers] or a short class-name representation with address otherwise.
     *
     * Upstream:
     *   override fun toString(): String = toStringInternalImpl() ?: "$classSimpleName@$hexAddress"
     */
    std::string to_string() const override;

    /**
     * Upstream:
     *   override fun limitedParallelism(parallelism: Int, name: String?): CoroutineDispatcher {
     *       parallelism.checkParallelism()
     *       return namedOrThis(name)
     *   }
     */
    std::shared_ptr<CoroutineDispatcher> limited_parallelism(
        int parallelism,
        const std::string& name = "") override;

protected:
    /**
     * Internal method for more specific [to_string] implementations. It returns a non-empty
     * optional if this dispatcher is set in the platform as the main one.
     *
     * Upstream:
     *   @InternalCoroutinesApi
     *   protected fun toStringInternalImpl(): String? { ... }
     */
    std::optional<std::string> to_string_internal_impl() const;
};

} // namespace kotlinx::coroutines
