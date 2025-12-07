// Original: kotlinx-coroutines-core/common/test/ImmediateYieldTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle suspend functions and dispatcher mechanics
// TODO: Implement CoroutineDispatcher and ContinuationInterceptor

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.test.*

class ImmediateYieldTest : public TestBase {
public:
    // See https://github.com/Kotlin/kotlinx.coroutines/issues/1474
    // TODO: @Test
    void test_immediate_yield() {
        // TODO: runTest {
        expect(1);
        // TODO: launch(ImmediateDispatcher(coroutineContext[ContinuationInterceptor])) {
            expect(2);
            // TODO: yield();
            expect(4);
        // }
        expect(3); // after yield
        // TODO: yield(); // yield back
        finish(5);
        // TODO: }
    }

private:
    // imitate immediate dispatcher
    class ImmediateDispatcher : public CoroutineDispatcher {
    private:
        CoroutineDispatcher* delegate;

    public:
        ImmediateDispatcher(ContinuationInterceptor* job)
            : delegate(static_cast<CoroutineDispatcher*>(job)) {}

        bool is_dispatch_needed(const CoroutineContext& context) const override {
            return false;
        }

        void dispatch(const CoroutineContext& context, Runnable* block) override {
            delegate->dispatch(context, block);
        }
    };

public:
    // TODO: @Test
    void test_wrapped_unconfined_dispatcher_yield() {
        // TODO: runTest {
        expect(1);
        // TODO: launch(wrapperDispatcher(Dispatchers.Unconfined)) {
            expect(2);
            // TODO: yield(); // Would not work with wrapped unconfined dispatcher
            expect(3);
        // }
        finish(4); // after launch
        // TODO: }
    }

    // TODO: @Test
    void test_wrapped_unconfined_dispatcher_yield_stack_overflow() {
        // TODO: runTest {
        expect(1);
        // TODO: withContext(wrapperDispatcher(Dispatchers.Unconfined)) {
        //     repeat(100000) {
        //         yield();
        //     }
        // }
        finish(2);
        // TODO: }
    }
};

} // namespace coroutines
} // namespace kotlinx
