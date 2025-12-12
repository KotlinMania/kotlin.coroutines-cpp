// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.internal.TestMainDispatcher
// Original package: kotlinx.coroutines.test.internal
//
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: Kotlin visibility needs C++ equivalent
// TODO: Kotlin lazy delegation pattern needs C++ equivalent
// TODO: Annotations (@Suppress) preserved as comments
// TODO: Kotlin atomic operations (kotlinx.atomicfu) need C++ std::atomic equivalents
// TODO: expect function needs platform-specific implementation

#include <atomic>
#include <functional>
#include <stdexcept>
#include <string>

namespace kotlinx {
    namespace coroutines {
        namespace test {
            namespace {
                // package kotlinx.coroutines.test.internal

                // import kotlinx.atomicfu.*
                // import kotlinx.coroutines.*
                // import kotlinx.coroutines.test.*
                // import kotlin.coroutines.*

                /**
 * The testable main dispatcher used by kotlinx-coroutines-test.
 * It is a [MainCoroutineDispatcher] that delegates all actions to a settable delegate.
 */
                class TestMainDispatcher : MainCoroutineDispatcher, Delay {
                private:
                    std::function<CoroutineDispatcher*()> create_inner_main_;
                    CoroutineDispatcher *main_dispatcher_;
                    bool main_dispatcher_initialized_;
                    NonConcurrentlyModifiable<CoroutineDispatcher *> *delegate_;

                public:
                    TestMainDispatcher(std::function<CoroutineDispatcher*()> create_inner_main)
                        : create_inner_main_(create_inner_main),
                          main_dispatcher_(nullptr),
                          main_dispatcher_initialized_(false),
                          delegate_(new NonConcurrentlyModifiable<CoroutineDispatcher *>(nullptr, "Dispatchers.Main")) {
                    }

                    TestMainDispatcher(CoroutineDispatcher *delegate)
                        : TestMainDispatcher([delegate]() { return delegate; }) {
                    }

                private:
                    CoroutineDispatcher *get_main_dispatcher() {
                        if (!main_dispatcher_initialized_) {
                            main_dispatcher_ = create_inner_main_();
                            main_dispatcher_initialized_ = true;
                        }
                        return main_dispatcher_;
                    }

                    CoroutineDispatcher *dispatcher() {
                        auto *d = delegate_->value();
                        return d != nullptr ? d : get_main_dispatcher();
                    }

                    Delay *delay() {
                        auto *d = dispatcher();
                        auto *delay_impl = dynamic_cast<Delay *>(d);
                        return delay_impl != nullptr ? delay_impl : &default_delay;
                    }

                public:
                    MainCoroutineDispatcher *immediate() override {
                        auto *d = dispatcher();
                        auto *main_disp = dynamic_cast<MainCoroutineDispatcher *>(d);
                        if (main_disp != nullptr) {
                            return main_disp->immediate();
                        }
                        return this;
                    }

                    void dispatch(const CoroutineContext &context, Runnable *block) override {
                        dispatcher()->dispatch(context, block);
                    }

                    bool is_dispatch_needed(const CoroutineContext &context) const override {
                        return dispatcher()->is_dispatch_needed(context);
                    }

                    void dispatch_yield(const CoroutineContext &context, Runnable *block) override {
                        dispatcher()->dispatch_yield(context, block);
                    }

                    void set_dispatcher(CoroutineDispatcher *disp) {
                        delegate_->set_value(disp);
                    }

                    void reset_dispatcher() {
                        delegate_->set_value(nullptr);
                    }

                    void schedule_resume_after_delay(int64_t time_millis,
                                                     CancellableContinuation<void> *continuation) override {
                        delay()->schedule_resume_after_delay(time_millis, continuation);
                    }

                    DisposableHandle *invoke_on_timeout(int64_t time_millis, Runnable *block,
                                                        const CoroutineContext &context) override {
                        return delay()->invoke_on_timeout(time_millis, block, context);
                    }

                    static TestDispatcher *current_test_dispatcher() {
                        auto *main = Dispatchers::Main;
                        auto *test_main = dynamic_cast<TestMainDispatcher *>(main);
                        if (test_main == nullptr) return nullptr;
                        auto *delegate_val = test_main->delegate_->value();
                        return dynamic_cast<TestDispatcher *>(delegate_val);
                    }

                    static TestCoroutineScheduler *current_test_scheduler() {
                        auto *disp = current_test_dispatcher();
                        return disp != nullptr ? &disp->scheduler() : nullptr;
                    }

                private:
                    /**
     * A wrapper around a value that attempts to throw when writing happens concurrently with reading.
     *
     * The read operations never throw. Instead, the failures detected inside them will be remembered and thrown on the
     * next modification.
     */
                    template<typename T>
                    class NonConcurrentlyModifiable {
                    private:
                        std::atomic<void *> reader_; // last reader to attempt access (stored as Throwable*)
                        std::atomic<int> readers_; // number of concurrent readers
                        std::atomic<void *> writer_;
                        // writer currently performing value modification (stored as Throwable*)
                        std::atomic<void *> exception_when_reading_; // exception from reading (stored as Throwable*)
                        std::atomic<T> value_; // the backing field for the value
                        std::string name_;

                        std::logic_error concurrent_ww(void *location) const {
                            return std::logic_error(name_ + " is modified concurrently");
                        }

                        std::logic_error concurrent_rw(void *location) const {
                            return std::logic_error(name_ + " is used concurrently with setting it");
                        }

                    public:
                        NonConcurrentlyModifiable(T initial_value, const std::string &name)
                            : reader_(nullptr), readers_(0), writer_(nullptr),
                              exception_when_reading_(nullptr), value_(initial_value), name_(name) {
                        }

                        T value() {
                            // TODO: Kotlin Throwable("reader location") for stack traces
                            void *reader_location = reinterpret_cast<void *>(1); // placeholder
                            reader_.store(reader_location);
                            readers_.fetch_add(1);

                            void *writer_val = writer_.load();
                            if (writer_val != nullptr) {
                                exception_when_reading_.store(reinterpret_cast<void *>(1)); // placeholder exception
                            }

                            T result = value_.load();
                            readers_.fetch_sub(1);
                            return result;
                        }

                        void set_value(T new_value) {
                            void *exception = exception_when_reading_.exchange(nullptr);
                            if (exception != nullptr) {
                                throw std::logic_error("Concurrent read/write detected");
                            }

                            if (readers_.load() != 0) {
                                void *reader_val = reader_.load();
                                if (reader_val != nullptr) {
                                    throw concurrent_rw(reader_val);
                                }
                            }

                            // TODO: Kotlin Throwable("other writer location") for stack traces
                            void *writer_location = reinterpret_cast<void *>(2); // placeholder
                            void *prev_writer = writer_.exchange(writer_location);
                            if (prev_writer != nullptr) {
                                throw concurrent_ww(prev_writer);
                            }

                            value_.store(new_value);
                            writer_.compare_exchange_strong(writer_location, nullptr);

                            if (readers_.load() != 0) {
                                void *reader_val = reader_.load();
                                if (reader_val != nullptr) {
                                    throw concurrent_rw(reader_val);
                                }
                            }
                        }
                    };

                    static Delay default_delay;
                };

                // @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE") // do not remove the INVISIBLE_REFERENCE suppression: required in K2
                // TODO: DefaultDelay is an implementation detail
                Delay TestMainDispatcher::default_delay; // placeholder

                // @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE") // do not remove the INVISIBLE_REFERENCE suppression: required in K2
                // TODO: expect function - needs platform-specific implementation
                TestMainDispatcher &get_test_main_dispatcher(Dispatchers & dispatchers);
            } // namespace internal
        } // namespace test
    } // namespace coroutines
} // namespace kotlinx