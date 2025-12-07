#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include <string>

namespace kotlinx {
namespace coroutines {

/**
 * Base class for special [CoroutineDispatcher] which is confined to application "Main" or "UI" thread
 * and used for any UI-based activities. Instance of `MainDispatcher` can be obtained by [Dispatchers.Main].
 */
class MainCoroutineDispatcher : public CoroutineDispatcher {
public:
    virtual ~MainCoroutineDispatcher() = default;

    /**
     * Returns dispatcher that executes coroutines immediately when it is already in the right context
     * (e.g. current looper is the same as this handler's looper) without an additional [re-dispatch][CoroutineDispatcher.dispatch].
     */
    virtual MainCoroutineDispatcher& get_immediate() = 0;

    /**
     * Returns a name of this main dispatcher for debugging purposes.
     */
    std::string to_string() const override;

    CoroutineDispatcher& limited_parallelism(int parallelism, const std::string* name = nullptr) override;

protected:
    /**
     * Internal method for more specific [tostd::string] implementations.
     */
    std::string to_string_internal_impl() const;
};

} // namespace coroutines
} // namespace kotlinx
