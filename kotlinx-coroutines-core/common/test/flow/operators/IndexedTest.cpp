// Original file: kotlinx-coroutines-core/common/test/flow/operators/IndexedTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class IndexedTest : public TestBase {
public:
    // @Test
    void test_with_index() /* TODO: = runTest */ {
        auto flow = flow_of(3, 2, 1).with_index();
        assert_equals(
            std::vector<IndexedValue<int>>{IndexedValue(0, 3), IndexedValue(1, 2), IndexedValue(2, 1)},
            flow.to_list()
        );
    }

    // @Test
    void test_with_index_empty() /* TODO: = runTest */ {
        auto flow = empty_flow<int>().with_index();
        assert_equals(std::vector<IndexedValue<int>>{}, flow.to_list());
    }

    // @Test
    void test_collect_indexed() /* TODO: = runTest */ {
        std::vector<IndexedValue<int64_t>> result;
        flow_of(3L, 2L, 1L).collect_indexed([&result](int index, int64_t value) {
            result.push_back(IndexedValue(index, value));
        });
        assert_equals(
            std::vector<IndexedValue<int64_t>>{IndexedValue(0, 3L), IndexedValue(1, 2L), IndexedValue(2, 1L)},
            result
        );
    }

    // @Test
    void test_collect_indexed_empty_flow() /* TODO: = runTest */ {
        auto flow = flow<int>([]() /* TODO: suspend */ {
            expect(1);
        });

        flow.collect_indexed([](int, int) {
            expect_unreached();
        });

        finish(2);
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
