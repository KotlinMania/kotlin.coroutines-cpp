// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/DelayDurationTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED", "DEPRECATION")
// KT-21913

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*
// TODO: import kotlin.time.*
// TODO: import kotlin.time.Duration.Companion.seconds
// TODO: import kotlin.time.Duration.Companion.nanoseconds

class DelayDurationTest : public TestBase {
public:
    // @Test
    void test_cancellation() {
        run_test([](auto it) { return dynamic_cast<CancellationException*>(it) != nullptr; },
        [this]() {
            run_and_cancel(Duration::seconds(1));
        });
    }

    // @Test
    void test_infinite() {
        run_test([](auto it) { return dynamic_cast<CancellationException*>(it) != nullptr; },
        [this]() {
            run_and_cancel(Duration::kInfinite);
        });
    }

    // @Test
    void test_regular_delay() {
        run_test([this]() {
            auto deferred = async([this]() {
                expect(2);
                delay(Duration::seconds(1));
                expect(4);
            });

            expect(1);
            yield();
            expect(3);
            deferred.await();
            finish(5);
        });
    }

    // @Test
    void test_nano_delay() {
        run_test([this]() {
            auto deferred = async([this]() {
                expect(2);
                delay(Duration::nanoseconds(1));
                expect(4);
            });

            expect(1);
            yield();
            expect(3);
            deferred.await();
            finish(5);
        });
    }

private:
    void run_and_cancel(Duration time) {
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
