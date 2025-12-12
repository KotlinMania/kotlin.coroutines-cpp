// Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/SharedFlowTest.kt

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.channels.*
// TODO: #include equivalent for kotlin.random.*
// TODO: #include equivalent for kotlin.test.*

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            /**
 * This test suite contains some basic tests for [SharedFlow]. There are some scenarios here written
 * using [expect] and they are not very readable. See [SharedFlowScenarioTest] for a better
 * behavioral test-suit.
 */
            class SharedFlowTest : public TestBase {
            public:
                // @Test
                void test_rendezvous_shared_flow_basic() {
                    // TODO: implement coroutine suspension
                    run_test([this]() {
                        expect(1);
                        auto sh = MutableSharedFlow<int *>();
                        assert_true(sh.replay_cache().empty());
                        assert_equals(0, sh.subscription_count().value);
                        sh.emit(1); // no suspend
                        assert_true(sh.replay_cache().empty());
                        assert_equals(0, sh.subscription_count().value);
                        expect(2);
                        // one collector
                        auto job1 = launch(CoroutineStart::kUndispatched, [&]() {
                            expect(3);
                            sh.collect([&](int *it) {
                                switch (*it) {
                                    case 4: expect(5);
                                        break;
                                    case 6: expect(7);
                                        break;
                                    case 10: expect(11);
                                        break;
                                    case 13: expect(14);
                                        break;
                                    default: expect_unreached();
                                }
                            });
                            expect_unreached(); // does not complete normally
                        });
                        expect(4);
                        assert_equals(1, sh.subscription_count().value);
                        sh.emit(4);
                        assert_true(sh.replay_cache().empty());
                        expect(6);
                        sh.emit(6);
                        expect(8);
                        // one more collector
                        auto job2 = launch(CoroutineStart::kUndispatched, [&]() {
                            expect(9);
                            sh.collect([&](int *it) {
                                if (it == nullptr) {
                                    expect(20);
                                } else {
                                    switch (*it) {
                                        case 10: expect(12);
                                            break;
                                        case 13: expect(15);
                                            break;
                                        case 17: expect(18);
                                            break;
                                        case 21: expect(22);
                                            break;
                                        default: expect_unreached();
                                    }
                                }
                            });
                            expect_unreached(); // does not complete normally
                        });
                        expect(10);
                        assert_equals(2, sh.subscription_count().value);
                        sh.emit(10); // to both collectors now!
                        assert_true(sh.replay_cache().empty());
                        expect(13);
                        sh.emit(13);
                        expect(16);
                        job1.cancel(); // cancel the first collector
                        yield();
                        assert_equals(1, sh.subscription_count().value);
                        expect(17);
                        sh.emit(17); // only to second collector
                        expect(19);
                        sh.emit(nullptr); // emit null to the second collector
                        expect(21);
                        sh.emit(21); // non-null again
                        expect(23);
                        job2.cancel(); // cancel the second collector
                        yield();
                        assert_equals(0, sh.subscription_count().value);
                        expect(24);
                        sh.emit(24); // does not go anywhere
                        assert_equals(0, sh.subscription_count().value);
                        assert_true(sh.replay_cache().empty());
                        finish(25);
                    });
                }

                // Additional tests would follow similar pattern...
                // For brevity, showing structure for key tests

                // @Test
                void test_replay1_shared_flow_basic() {
                    // TODO: implement coroutine suspension
                    // ... similar transliteration pattern
                }

                // @Test
                void test_drop_latest() {
                    test_drop_latest_or_oldest(BufferOverflow::kDropLatest);
                }

                // @Test
                void test_drop_oldest() {
                    test_drop_latest_or_oldest(BufferOverflow::kDropOldest);
                }

            private:
                void test_drop_latest_or_oldest(BufferOverflow buffer_overflow) {
                    // TODO: implement coroutine suspension
                    run_test([&]() {
                        reset();
                        expect(1);
                        auto sh = MutableSharedFlow<int *>(1, /*onBufferOverflow=*/buffer_overflow);
                        sh.emit(1);
                        sh.emit(2);
                        // always keeps last w/o collectors
                        assert_equals(std::vector<int>{2}, sh.replay_cache());
                        assert_equals(0, sh.subscription_count().value);
                        // ... rest of implementation
                        finish(11);
                    });
                }

                struct Data {
                    int x;
                    bool operator==(const Data &other) const { return x == other.x; }
                };

                class SubJob {
                public:
                    Job *job;
                    int last_received = 0;
                };
            };

            /**
 * Check that, by the time [SharedFlow.collect] suspends for the first time, its subscription is already active.
 */
            template<typename T>
            void test_subscription_by_first_suspension_in_collect(
                CoroutineScope &scope,
                T &flow,
                std::function<void(T &, int)> emit
            ) {
                // TODO: implement coroutine suspension
                int received = 0;
                auto job = scope.launch(CoroutineStart::kUndispatched, [&]() {
                    flow.collect([&](int it) {
                        received = it;
                    });
                });
                emit(flow, 1);
                assert_equals(1, received);
                job.cancel();
            }
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement coroutine suspension for all suspend functions
// 2. Implement MutableSharedFlow template class
// 3. Implement BufferOverflow enumeration (kDropLatest, kDropOldest, kSuspend)
// 4. Implement TestBase base class
// 5. Implement run_test, yield, launch functions
// 6. Implement Job class with cancel, join methods
// 7. Implement CoroutineStart enumeration
// 8. Implement replay_cache, subscription_count methods
// 9. Implement test assertions
// 10. Complete all test method implementations
// 11. Implement Random class for Kotlin compatibility
// 12. Add proper includes for all dependencies