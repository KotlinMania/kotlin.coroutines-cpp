// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/flow/terminal/ToCollectionTest.kt
// TODO(port): runTest wrapper not yet wired for suspend semantics
// TODO(port): toList/toSet terminal operators need implementation

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Collection.hpp"
#include <vector>
#include <set>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class ToCollectionTest : public TestBase {
private:
    std::shared_ptr<Flow<int>> make_test_flow() {
        return flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
            for (int i = 0; i < 10; ++i) {
                collector->emit(42, cont);
            }
            return nullptr;
        });
    }

    std::shared_ptr<Flow<int>> make_empty_flow() {
        return empty_flow<int>();
    }

public:
    // @Test
    void test_to_list() {
        // TODO(port): runTest { ... }
        auto f = make_test_flow();
        // TODO(port): to_list(f) terminal operator
        // Expected: List of 10 elements, all 42
        // auto result = to_list(f);
        // assert(result.size() == 10);
        // for (auto& v : result) assert(v == 42);

        // auto empty_result = to_list(make_empty_flow());
        // assert(empty_result.empty());
    }

    // @Test
    void test_to_set() {
        // TODO(port): runTest { ... }
        auto f = make_test_flow();
        // TODO(port): to_set(f) terminal operator
        // Expected: Set with single element 42
        // auto result = to_set(f);
        // assert(result.size() == 1);
        // assert(result.count(42) == 1);

        // auto empty_result = to_set(make_empty_flow());
        // assert(empty_result.empty());
    }
};

} // namespace coroutines
} // namespace kotlinx
