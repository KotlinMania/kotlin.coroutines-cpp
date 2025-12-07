// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlattenMergeTest.kt
// TODO: handle imports (kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class FlattenMergeTest : public FlatMapMergeBaseTest {
public:
    // TODO: translate generic member function template
    template<typename T>
    Flow<T> flat_map(Flow<T>& flow, std::function<Flow<T>(T)> mapper) override {
        return flow.map(mapper).flatten_merge();
    }

    // @Test
    // TODO: translate @Test annotation
    void test_flat_map_concurrency() /* TODO: = runTest */ {
        int concurrent_requests = 0;
        auto flow_lambda = [&concurrent_requests](int value) {
            return flow([&, value]() {
                ++concurrent_requests;
                emit(value);
                delay(LONG_MAX);
            });
        };

        auto flow = range(1, 100)
            .as_flow()
            .map(flow_lambda)
            .flatten_merge(/* concurrency = */ 2);

        auto consumer = launch([&]() /* TODO: suspend */ {
            flow.collect([&](int value) {
                expect(value);
            });
        });

        for (int i = 0; i < 4; ++i) {
            yield();
        }

        assert_equals(2, concurrent_requests);
        consumer.cancel_and_join();
        finish(3);
    }

    // @Test
    void test_context_preservation_across_flows() /* TODO: = runTest */ {
        auto result = flow([]() /* TODO: suspend */ {
            flow_of(1, 2).flat_map_merge([](int it) {
                return flow([it]() /* TODO: suspend */ {
                    yield();
                    emit(it);
                });
            }).collect([](int it) {
                emit(it);
            });
        }).to_list();
        assert_equals(list_of(1, 2), result);
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
