// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/BuilderContractsTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlinx.coroutines.channels.*
        // TODO: import kotlinx.coroutines.selects.*
        // TODO: import kotlin.test.*

        class BuilderContractsTest : public TestBase {
        public:
            // @Test
            // TODO: Translate @Test annotation
            void test_contracts() {
                run_test([this]() {
                    // Coroutine scope
                    int cs;
                    coroutine_scope([&cs]() {
                        cs = 42;
                    });
                    consume(cs);

                    // Supervisor scope
                    int svs;
                    supervisor_scope([&svs]() {
                        svs = 21;
                    });
                    consume(svs);

                    // with context scope
                    int wctx;
                    with_context(Dispatchers::Unconfined, [&wctx]() {
                        wctx = 239;
                    });
                    consume(wctx);

                    int wt;
                    with_timeout(LONG_MAX, [&wt]() {
                        wt = 123;
                    });
                    consume(wt);

                    int s;
                    select<void>([&s]() {
                        s = 42;
                        auto job = Job();
                        job.complete();
                        job.on_join([]() {
                        });
                    });
                    consume(s);


                    int ch;
                    auto i = Channel<int>();
                    i.consume([&ch]() {
                        ch = 321;
                    });
                    consume(ch);
                });
            }

        private:
            void consume(int a) {
                /*
         * Verify the value is actually set correctly
         * (non-zero, VerificationError is not triggered, can be read)
         */
                assert_not_equals(0, a);
                assert_equals(std::hash<int>{}(a), a);
            }
        };
    } // namespace coroutines
} // namespace kotlinx