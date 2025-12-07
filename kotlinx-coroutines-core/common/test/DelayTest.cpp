// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/DelayTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED", "DEPRECATION") // KT-21913

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

class DelayTest : public TestBase {
public:
    // @Test
    void test_cancellation() {
        run_test([](auto it) { return dynamic_cast<CancellationException*>(it) != nullptr; },
        [this]() {
            run_and_cancel(1000);
        });
    }

    // @Test
    void test_max_long_value() {
        run_test([](auto it) { return dynamic_cast<CancellationException*>(it) != nullptr; },
        [this]() {
            run_and_cancel(LONG_MAX);
        });
    }

    // @Test
    void test_max_int_value() {
        run_test([](auto it) { return dynamic_cast<CancellationException*>(it) != nullptr; },
        [this]() {
            run_and_cancel(static_cast<int64_t>(INT_MAX));
        });
    }

    // @Test
    void test_regular_delay() {
        run_test([this]() {
            auto deferred = async([this]() {
                expect(2);
                delay(1);
                expect(3);
            });

            expect(1);
            yield();
            deferred.await();
            finish(4);
        });
    }

private:
    void run_and_cancel(int64_t time) {
        coroutine_scope([this, time]() {
            expect(1);
            auto deferred = async([this, time]() {
                expect(2);
                delay(time);
                expect_unreached();
            });

            yield();
            expect(3);
            require(deferred.is_active());
            deferred.cancel();
            finish(4);
            deferred.await();
        });
    }
};

} // namespace coroutines
} // namespace kotlinx
