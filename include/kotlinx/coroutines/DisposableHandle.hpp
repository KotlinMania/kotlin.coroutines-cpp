#pragma once

namespace kotlinx {
namespace coroutines {

/**
 * A handle to a disposabled resource.
 */
struct DisposableHandle {
    virtual ~DisposableHandle() = default;
    
    /**
     * Disposes the resource.
     */
    virtual void dispose() = 0;
};

// NoOp Implementation available globally?
struct NoOpDisposableHandle : public DisposableHandle {
    void dispose() override {}
    static NoOpDisposableHandle* instance() {
        static NoOpDisposableHandle inst;
        return &inst;
    }
};

} // namespace coroutines
} // namespace kotlinx
