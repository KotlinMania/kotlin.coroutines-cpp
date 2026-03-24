// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/flow/terminal/FoldTest.kt
// TODO(port): runTest wrapper not yet wired for suspend semantics

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Reduce.hpp"

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

class FoldTest : public TestBase {
public:
    // @Test
    void test_fold() {
        // TODO(port): runTest { ... }
        auto f = flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
            collector->emit(1, cont);
            collector->emit(2, cont);
            return nullptr;
        });

        void* result = fold<int, int>(f, 3, [](int acc, int value) { return acc + value; }, nullptr);
        assert(*static_cast<int*>(result) == 6);
        delete static_cast<int*>(result);
    }

    // @Test
    void test_empty_fold() {
        // TODO(port): runTest { ... }
        auto f = empty_flow<int>();
        void* result = fold<int, int>(f, 42, [](int acc, int value) { return acc + value; }, nullptr);
        assert(*static_cast<int*>(result) == 42);
        delete static_cast<int*>(result);
    }

    // @Test
    void test_error_cancels_upstream() {
        // TODO(port): runTest { ... } — requires coroutineScope, launch, Channel
        // TODO(port): Channel<Unit> latch pattern requires channel infrastructure
        // TODO(semantics): coroutineScope + launch within flow builder
        expect(1);

        auto f = flow::flow<int>([this](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
            // TODO(port): coroutineScope {
            //     launch {
            //         latch.send(Unit)
            //         expect(3)
            //         hang { expect(5) }
            //     }
            expect(2);
            collector->emit(1, cont);
            // }
            return nullptr;
        });

        bool caught = false;
        try {
            fold<int, int>(f, 42, [this](int /*acc*/, int /*value*/) -> int {
                // TODO(port): latch.receive()
                expect(4);
                throw TestException();
                return 0;
            }, nullptr);
        } catch (const TestException&) {
            caught = true;
        }
        assert(caught);
        finish(6);
    }
};

} // namespace coroutines
} // namespace kotlinx
