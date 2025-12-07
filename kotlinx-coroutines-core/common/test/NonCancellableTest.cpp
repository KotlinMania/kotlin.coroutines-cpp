// Original: kotlinx-coroutines-core/common/test/NonCancellableTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle NonCancellable context behavior
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

class NonCancellableTest : public TestBase {
public:
    // TODO: @Test
    void test_non_cancellable() {
        // TODO: runTest {
        expect(1);
        // TODO: const auto job = async {
        //     with_context(NonCancellable) {
                expect(2);
        //         yield();
                expect(4);
        //     }

            expect(5);
        //     yield();
        //     expect_unreached();
        // }

        // TODO: yield();
        // TODO: job.cancel();
        expect(3);
        // TODO: assertTrue(job.is_cancelled());
        // TODO: try {
        //     job.await();
        //     expect_unreached();
        // } catch (const JobCancellationException& e) {
        //     if (RECOVER_STACK_TRACES) {
        //         const auto cause = static_cast<const JobCancellationException&>(*e.cause); // shall be recovered JCE
        //         assertNull(cause.cause);
        //     } else {
        //         assertNull(e.cause);
        //     }
            finish(6);
        // }
        // TODO: }
    }

    // TODO: @Test
    void test_non_cancellable_with_exception() {
        // TODO: runTest {
        expect(1);
        // TODO: const auto deferred = async(NonCancellable) {
        //     with_context(NonCancellable) {
                expect(2);
        //         yield();
                expect(4);
        //     }

            expect(5);
        //     yield();
        //     expect_unreached();
        // }

        // TODO: yield();
        // TODO: deferred.cancel(TestCancellationException("TEST"));
        expect(3);
        // TODO: assertTrue(deferred.is_cancelled());
        // TODO: try {
        //     deferred.await();
        //     expect_unreached();
        // } catch (const TestCancellationException& e) {
        //     assertEquals("TEST", e.message);
            finish(6);
        // }
        // TODO: }
    }

    // TODO: @Test
    void test_non_cancellable_finally() {
        // TODO: runTest {
        expect(1);
        // TODO: const auto job = async {
        //     try {
                expect(2);
        //         yield();
        //         expect_unreached();
        //     } finally {
        //         with_context(NonCancellable) {
                    expect(4);
        //             yield();
                    expect(5);
        //         }
        //     }

        //     expect_unreached();
        // }

        // TODO: yield();
        // TODO: job.cancel();
        expect(3);
        // TODO: assertTrue(job.is_cancelled());

        // TODO: try {
        //     job.await();
        //     expect_unreached();
        // } catch (const CancellationException& e) {
            finish(6);
        // }
        // TODO: }
    }

    // TODO: @Test
    void test_non_cancellable_twice() {
        // TODO: runTest {
        expect(1);
        // TODO: const auto job = async {
        //     with_context(NonCancellable) {
                expect(2);
        //         yield();
                expect(4);
        //     }

        //     with_context(NonCancellable) {
                expect(5);
        //         yield();
                expect(6);
        //     }
        // }

        // TODO: yield();
        // TODO: job.cancel();
        expect(3);
        // TODO: assertTrue(job.is_cancelled());
        // TODO: try {
        //     job.await();
        //     expect_unreached();
        // } catch (const JobCancellationException& e) {
        //     if (RECOVER_STACK_TRACES) {
        //         const auto cause = static_cast<const JobCancellationException&>(*e.cause); // shall be recovered JCE
        //         assertNull(cause.cause);
        //     } else {
        //         assertNull(e.cause);
        //     }
            finish(7);
        // }
        // TODO: }
    }
};

} // namespace coroutines
} // namespace kotlinx
