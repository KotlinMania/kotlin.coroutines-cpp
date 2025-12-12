// Original file: kotlinx-coroutines-test/common/test/UnconfinedTestDispatcherTest.kt
// TODO: Remove or convert import statements
// TODO: Convert @Test annotations to appropriate test framework
// TODO: Convert suspend functions and coroutine builders
// TODO: Handle local class definitions
// TODO: Convert Flow operations (callbackFlow, combine, collect, etc.)

namespace kotlinx {
    namespace coroutines {
        namespace test {
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.channels.*
            // TODO: import kotlinx.coroutines.flow.*
            // TODO: import kotlin.test.*

            class UnconfinedTestDispatcherTest {
            public:
                // TODO: @Test
                void reproducer1742() {
                    // Local class definition
                    template
                    <
                    typename T >
                    class ObservableValue {
                    private:
                        T value_;
                        std::vector<std::function<void(T)> > listeners_;

                    public:
                        ObservableValue(T initial) : value_(initial) {
                        }

                        T value() const { return value_; }

                        void set(T value) {
                            value_ = value;
                            for (auto &listener: listeners_) {
                                listener(value);
                            }
                        }

                        void add_listener(std::function<void(T)> listener) {
                            listeners_.push_back(listener);
                        }

                        void remove_listener(std::function<void(T)> listener) {
                            // TODO: Need proper function comparison
                            // listeners.remove(listener);
                        }
                    };

                    auto observe = [](auto &observable_value) -> Flow<decltype(observable_value.value())> {
                        return callback_flow([&](auto &producer) {
                            auto listener = [&](auto value) {
                                if (!producer.is_closed_for_send()) {
                                    producer.try_send(value);
                                }
                            };
                            observable_value.add_listener(listener);
                            listener(observable_value.value());
                            producer.await_close([&]() { observable_value.remove_listener(listener); });
                        });
                    };

                    ObservableValue<int> int_provider(0);
                    ObservableValue<std::string> string_provider("");
                    std::pair<int, std::string> data{0, ""};
                    CoroutineScope scope(UnconfinedTestDispatcher());
                    scope.launch([&]() {
                        combine(
                            observe(int_provider),
                            observe(string_provider),
                            [](int int_value, std::string string_value) {
                                return std::make_pair(int_value, string_value);
                            }
                        ).collect([&](auto pair) {
                            data = pair;
                        });
                    });

                    int_provider.set(1);
                    string_provider.set("3");
                    int_provider.set(2);
                    int_provider.set(3);

                    scope.cancel();
                    assert(data == std::make_pair(3, std::string("3")));
                }

                // TODO: @Test
                void reproducer2082() {
                    run_test([&]() {
                        MutableStateFlow<int> subject1(1);
                        MutableStateFlow<std::string> subject2("a");
                        std::vector<std::pair<int, std::string> > values;

                        auto job = launch(UnconfinedTestDispatcher(test_scheduler), [&]() {
                            combine(subject1, subject2, [](int int_val, std::string str_val) {
                                return std::make_pair(int_val, str_val);
                            }).collect([&](auto it) {
                                delay(10000);
                                values.push_back(it);
                            });
                        });

                        subject1.value = 2;
                        delay(10000);
                        subject2.value = "b";
                        delay(10000);

                        subject1.value = 3;
                        delay(10000);
                        subject2.value = "c";
                        delay(10000);
                        delay(10000);
                        delay(1);

                        job.cancel();

                        using Pair = std::pair<int, std::string>;
                        std::vector<Pair> expected = {
                            Pair(1, "a"), Pair(2, "a"), Pair(2, "b"), Pair(3, "b"), Pair(3, "c")
                        };
                        assert(values == expected);
                    });
                }

                // TODO: @Test
                void reproducer2405() {
                    create_test_result([&]() {
                        auto dispatcher = UnconfinedTestDispatcher();
                        bool collected_error = false;
                        with_context(dispatcher, [&]() {
                            flow([&]() { emit(1); })
                                    .combine(
                                        flow<std::string>([&]() { throw std::invalid_argument(""); }),
                                        [](int int_val, std::string string_val) {
                                            return std::to_string(int_val) + string_val;
                                        }
                                    )
                                    .catch_exception([&](auto e) { emit("error"); })
                                    .collect([&](auto it) {
                                        assert(it == "error");
                                        collected_error = true;
                                    });
                        });
                        assert(collected_error);
                    });
                }

                /** An example from the [UnconfinedTestDispatcher] documentation. */
                // TODO: @Test
                void test_unconfined_dispatcher() {
                    run_test([&]() {
                        std::vector<int> values;
                        MutableStateFlow<int> state_flow(0);
                        auto job = launch(UnconfinedTestDispatcher(test_scheduler), [&]() {
                            state_flow.collect([&](int it) {
                                values.push_back(it);
                            });
                        });
                        state_flow.value = 1;
                        state_flow.value = 2;
                        state_flow.value = 3;
                        job.cancel();
                        assert(values == std::vector<int>{0, 1, 2, 3});
                    });
                }

                /** Tests that child coroutines are eagerly entered. */
                // TODO: @Test
                void test_eagerly_entering_child_coroutines() {
                    run_test(UnconfinedTestDispatcher(), [&]() {
                        bool entered = false;
                        auto deferred = CompletableDeferred<void>();
                        bool completed = false;
                        launch([&]() {
                            entered = true;
                            deferred.await();
                            completed = true;
                        });
                        assert(entered); // `entered = true` already executed.
                        assert(!completed); // however, the child coroutine then suspended, so it is enqueued.
                        deferred.complete(); // resume the coroutine.
                        assert(completed); // now the child coroutine is immediately completed.
                    });
                }

                /** Tests that the [TestCoroutineScheduler] used for [Dispatchers.Main] gets used by default. */
                // TODO: @Test
                void test_scheduler_reuse() {
                    auto dispatcher1 = StandardTestDispatcher();
                    Dispatchers::set_main(dispatcher1);
                    try {
                        auto dispatcher2 = UnconfinedTestDispatcher();
                        // TODO: assertSame
                        assert(dispatcher1.scheduler == dispatcher2.scheduler);
                    }
                    finally{
                        Dispatchers::reset_main();
        
                    }
                }
            };
        } // namespace test
    } // namespace coroutines
} // namespace kotlinx