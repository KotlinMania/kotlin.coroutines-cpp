// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/AwaitCancellationTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

class AwaitCancellationTest : public TestBase {
public:
    // @Test
    // TODO: Translate @Test annotation
    void test_cancellation() {
        run_test([](auto it) { return dynamic_cast<CancellationException*>(it) != nullptr; },
        [this]() {
            expect(1);
            coroutine_scope([this]() {
                auto deferred = async<void>([this]() { // Deferred<Nothing>
                    expect(2);
                    await_cancellation();
                });
                yield();
                expect(3);
                require(deferred.is_active());
                deferred.cancel();
                finish(4);
                deferred.await();
            });
        });
    }
};

} // namespace coroutines
} // namespace kotlinx
