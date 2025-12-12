#pragma once
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/test/TestDispatcher.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace test {

/**
 * A coroutine scope for launching test coroutines.
 * It provides access to the [TestDispatcher] used for the test and
 * helper methods to control the virtual time.
 * 
 * @see runTest
 */
class TestScope : public CoroutineScope {
public:
    std::shared_ptr<CoroutineContext> context_;

    TestScope(std::shared_ptr<CoroutineContext> context) : context_(context) {}
    
    virtual ~TestScope() = default;

    std::shared_ptr<CoroutineContext> get_coroutine_context() const override {
        return context_;
    }
    
    // Additional test helpers would go here
};

} // namespace test
} // namespace coroutines
} // namespace kotlinx
