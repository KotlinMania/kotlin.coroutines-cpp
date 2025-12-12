// Original: kotlinx-coroutines-core/concurrent/test/DefaultDispatchersConcurrencyTest.kt
// TODO: Remove or convert import statements
// TODO: Convert class inheritance
// TODO: Implement Dispatchers.Default and Dispatchers.IO

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines

        class DefaultDispatcherConcurrencyTest : public AbstractDispatcherConcurrencyTest {
        public:
            CoroutineDispatcher *dispatcher() override {
                return &Dispatchers::Default;
            }
        };

        class IoDispatcherConcurrencyTest : public AbstractDispatcherConcurrencyTest {
        public:
            CoroutineDispatcher *dispatcher() override {
                return &Dispatchers::IO;
            }
        };
    } // namespace coroutines
} // namespace kotlinx