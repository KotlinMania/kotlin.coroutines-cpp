// Original file: kotlinx-coroutines-core/common/test/flow/operators/LintTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class LintTest : public TestBase {
public:
    // Tests that using SharedFlow::to_list and similar functions by passing a mutable collection does add values
    // to the provided collection.
    // @Test
    void test_shared_flow_to_collection() /* TODO: = runTest */ {
        auto shared_flow = MutableSharedFlow<int>();
        std::vector<int> list;
        std::set<int> set;
        auto jobs = std::vector<std::function<void()>>{
            [&]() /* TODO: suspend */ { shared_flow.to_list(list); },
            [&]() /* TODO: suspend */ { shared_flow.to_set(set); }
        };

        std::vector<Job> job_handles;
        for (auto& job_fn : jobs) {
            job_handles.push_back(launch(Dispatchers::Unconfined, job_fn));
        }

        for (int it = 0; it < 10; ++it) {
            shared_flow.emit(it);
        }

        for (auto& job : job_handles) {
            job.cancel_and_join();
        }

        std::vector<int> expected_list;
        for (int i = 0; i <= 9; ++i) expected_list.push_back(i);
        assert_equals(expected_list, list);

        std::set<int> expected_set;
        for (int i = 0; i <= 9; ++i) expected_set.insert(i);
        assert_equals(expected_set, set);
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
