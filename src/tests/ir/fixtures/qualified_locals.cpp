// NOTE(port): Compiler execution regression for declared C++ cv-qualification,
// ordinary object identity and destruction in Native-derived spill storage.
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include <cassert>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <type_traits>
#include <typeinfo>
#include <array>
#include <cstdint>
#include <limits>
using namespace kotlinx::coroutines;
enum class ConstantKind { VALUE = 3 };
enum class WideKind : __uint128_t {
    VALUE = (static_cast<__uint128_t>(1) << 100) + 37,
    MAXIMUM = ~static_cast<__uint128_t>(0)
};

int alive = 0;
int mode = 0;
int calls = 0;
int binding_evaluations = 0;
int array_alive = 0;
int array_copies = 0;
int array_destroyed[4]{};
int array_destructions = 0;
int array_sources = 0;
int branch_alive = 0;
int branch_constructed = 0;
int branch_destroyed = 0;
int empty_branch_inits = 0;
std::shared_ptr<Continuation<void*>> pending;
std::weak_ptr<Continuation<void*>> frame;
struct QualifiedValue {
    explicit QualifiedValue(int value) : value_(value) { ++alive; }
    QualifiedValue(const QualifiedValue&) = delete;
    ~QualifiedValue() { --alive; }
    int read() & { return -1000; }
    int read() const & { return value_; }
    int read() volatile & { return value_ + 1; }
    int read() const volatile & { return value_ + 2; }
private:
    int value_;
};
struct BranchGuard {
    explicit BranchGuard(int index) : index(index) { ++branch_alive; ++branch_constructed; }
    BranchGuard(const BranchGuard&) = delete;
    ~BranchGuard() { --branch_alive; ++branch_destroyed; }
    int index;
};
struct MemberParts {
    int first;
    unsigned second : 4;
};
struct ArrayValue {
    explicit ArrayValue(int value) : value(value) { ++array_alive; }
    ArrayValue(const ArrayValue& other) : value(other.value) {
        ++array_copies;
        if (mode == 5 && array_copies == 2) throw std::runtime_error("array copy failure");
        ++array_alive;
    }
    ~ArrayValue() {
        --array_alive;
        assert(array_destructions < 4);
        array_destroyed[array_destructions++] = value;
    }
    int value;
};
ArrayValue (&source_once(ArrayValue (&values)[2]))[2] {
    ++array_sources;
    return values;
}
struct TupleParts {
    int first;
    int second;
    template <std::size_t Index> int get() && {
        ++binding_evaluations;
        if constexpr (Index == 0) return first;
        else return second;
    }
};
namespace std {
template <> struct tuple_size<TupleParts> : integral_constant<size_t, 2> {};
template <size_t Index> struct tuple_element<Index, TupleParts> { using type = int; };
}
const QualifiedValue* fixed_identity = nullptr;
const volatile QualifiedValue* observed_identity = nullptr;
const volatile QualifiedValue* both_identity = nullptr;
int* started_identity = nullptr;
int* finished_identity = nullptr;
const int* cached_identity = nullptr;
int static_initializations = 0;
int initialize_static(int value) {
    ++static_initializations;
    return value;
}

[[clang::annotate("suspend")]]
void* await_value(std::shared_ptr<Continuation<void*>> completion) {
    ++calls;
    frame = completion;
    if (mode == 4) throw std::runtime_error("immediate failure");
    if (mode == 0) return new int(7);
    pending = std::move(completion);
    return intrinsics::get_COROUTINE_SUSPENDED();
}

bool ordinary_type_queries(std::shared_ptr<Continuation<void*>> completion) {
    using ResultType = decltype(await_value(completion));
    static_assert(std::is_same_v<ResultType, void*>);
    return sizeof(await_value(completion)) == sizeof(void*) &&
        !noexcept(await_value(completion)) &&
        typeid(await_value(completion)) == typeid(void*);
}

[[clang::annotate("suspend")]]
void* qualified_locals(int seed, std::shared_ptr<Continuation<void*>> completion) {
    using Value = QualifiedValue;
    using Fixed = const Value;
    typedef volatile Value Observed;
    using Both = const volatile Value;
    constexpr int count = 2;
    const int multiplier = 3;
    constexpr ConstantKind kind = ConstantKind::VALUE;
    constexpr std::uint64_t high = std::numeric_limits<std::uint64_t>::max();
    constexpr std::int64_t low = std::numeric_limits<std::int64_t>::min();
    constexpr __int128_t wide = (static_cast<__int128_t>(1) << 100) + 37;
    constexpr __int128_t wide_low = -(static_cast<__int128_t>(1) << 126) -
        (static_cast<__int128_t>(1) << 126);
    constexpr __uint128_t wide_high = ~static_cast<__uint128_t>(0);
    constexpr WideKind wide_kind = WideKind::VALUE;
    constexpr WideKind wide_kind_maximum = WideKind::MAXIMUM;
    const __int128_t* wide_identity = std::addressof(wide);
    std::array<int, count> constants{count, multiplier};
    const int* count_identity = std::addressof(count);
    static_assert(count == 2 && multiplier == 3);
    static_assert(std::integral_constant<int, count>::value == 2);
    static_assert(std::integral_constant<ConstantKind, kind>::value == ConstantKind::VALUE);
    static_assert(high == UINT64_MAX && low == INT64_MIN);
    static_assert(std::integral_constant<__int128_t, wide>::value ==
        (static_cast<__int128_t>(1) << 100) + 37);
    static_assert(std::integral_constant<__int128_t, wide_low>::value < -wide);
    static_assert(std::integral_constant<__uint128_t, wide_high>::value ==
        ~static_cast<__uint128_t>(0));
    static_assert(std::is_same_v<decltype(wide), const __int128_t>);
    static_assert(std::integral_constant<WideKind, wide_kind>::value == WideKind::VALUE);
    static_assert(std::integral_constant<WideKind, wide_kind_maximum>::value == WideKind::MAXIMUM);
    static_assert(std::is_same_v<decltype(count), const int>);
    const int initial = seed + 5;
    static int started = seed - 37, finished = initial - 42;
    static const int cached = initialize_static(initial + seed);
    assert(cached == 79 && static_initializations == 1);
    if (cached_identity) assert(cached_identity == std::addressof(cached));
    cached_identity = std::addressof(cached);
    if (started_identity) {
        assert(started_identity == std::addressof(started));
        assert(finished_identity == std::addressof(finished));
    }
    started_identity = std::addressof(started);
    finished_identity = std::addressof(finished);
    ++started;
    Fixed fixed(42);
    Observed observed(43);
    Both both(44);
    int items[2]{3, 4};
    auto& [head, tail] = items;
    auto [copy_head, copy_tail] = items;
    ++head;
    int* head_identity = std::addressof(head);
    int grid[2][2]{{1, 2}, {3, 4}};
    auto [first_row, second_row] = grid;
    grid[0][0] = 9;
    ArrayValue originals[2]{ArrayValue(11), ArrayValue(12)};
    auto [first_copy, second_copy] = source_once(originals);
    originals[0].value = 99;
    ArrayValue* copy_identity = std::addressof(first_copy);
    auto [member_first, member_second] = MemberParts{3, 4};
    ++member_second;
    auto [from_get, from_get_second] = TupleParts{3, 4};
    auto [resource, tag] = std::make_pair(std::make_unique<QualifiedValue>(9), 1);
    QualifiedValue* resource_identity = resource.get();
    static_assert(std::is_same_v<decltype(fixed), const QualifiedValue>);
    static_assert(std::is_same_v<decltype((fixed)), const QualifiedValue&>);
    static_assert(std::is_same_v<decltype(observed), volatile QualifiedValue>);
    static_assert(std::is_same_v<decltype(head), int>);
    static_assert(std::is_same_v<decltype((head)), int&>);
    static_assert(std::is_same_v<decltype(items), int[2]>);
    static_assert(std::is_same_v<decltype(first_row), int[2]>);
    static_assert(std::is_same_v<decltype(resource), std::unique_ptr<QualifiedValue>>);
    static_assert(std::is_same_v<decltype((resource)), std::unique_ptr<QualifiedValue>&>);
    static_assert(sizeof(await_value(completion)) == sizeof(void*));
    static_assert(!noexcept(await_value(completion)));
    static_assert(noexcept(head));
    static_assert(noexcept(resource.get()));
    assert(noexcept(head) && noexcept(resource.get()));
    assert(sizeof(dsl::suspend(await_value(completion))) == sizeof(void*));
    assert(binding_evaluations == 2);
    fixed_identity = std::addressof(fixed);
    observed_identity = std::addressof(observed);
    both_identity = std::addressof(both);
    int total = 0;
    for (int index = 0; index < 2; ++index) {
        using Value = int;
        Value increment = Value(7);
        if constexpr (++empty_branch_inits; false) {
            std::unique_ptr<int> unused(static_cast<int*>(dsl::suspend(await_value(completion))));
            assert(unused);
        }
        if constexpr (BranchGuard guard(index); count == 2) {
            assert(guard.index == index && branch_alive == 1);
            total += fixed.read() + observed.read() + both.read();
            void* raw = dsl::suspend(await_value(completion));
            std::unique_ptr<int> box(static_cast<int*>(raw));
            assert(std::addressof(fixed) == fixed_identity);
            assert(std::addressof(observed) == observed_identity);
            assert(std::addressof(both) == both_identity);
            assert(cached_identity == std::addressof(cached));
            assert(cached == 79 && static_initializations == 1);
            assert(*box == static_cast<Value>(increment));
            assert(std::addressof(head) == head_identity && head_identity == &items[0]);
            assert(head == 4 && tail == 4 && copy_head == 3 && copy_tail == 4);
            assert(first_row[0] == 1 && first_row[1] == 2 && second_row[0] == 3 && second_row[1] == 4);
            assert(first_copy.value == 11 && second_copy.value == 12);
            assert(std::addressof(first_copy) == copy_identity && copy_identity != &originals[0]);
            assert(array_copies == 2 && array_sources == 1);
            assert(std::addressof(count) == count_identity && *count_identity == 2);
            assert(std::addressof(wide) == wide_identity && *wide_identity == wide);
            assert(wide_low < -wide && wide_high == ~static_cast<__uint128_t>(0));
            assert(static_cast<__uint128_t>(wide_kind) == static_cast<__uint128_t>(wide));
            assert(static_cast<__uint128_t>(wide_kind_maximum) == wide_high);
            assert(constants[0] == count && constants[1] == multiplier);
            assert(member_first == 3 && member_second == 5);
            assert(from_get == 3 && from_get_second == 4 && binding_evaluations == 2);
            assert(resource.get() == resource_identity && tag == 1);
            total += fixed.read() + observed.read() + both.read() + *box;
        } else {
            struct DiscardedValue { int value; };
            DiscardedValue discarded{0};
            std::unique_ptr<int> unused(static_cast<int*>(dsl::suspend(await_value(completion))));
            assert(discarded.value == 0 && unused);
        }
    }
    ++finished;
    return new int(total);
}
struct Done final : Continuation<void*> {
    int resumes = 0;
    int value = 0;
    bool failed = false;
    bool cancelled = false;
    std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
    void resume_with(Result<void*> result) override {
        ++resumes;
        try {
            std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
            value = *box;
        } catch (const CancellationException&) { failed = cancelled = true; }
        catch (const std::runtime_error&) { failed = true; }
    }
};
int condition_alive = 0;
int condition_destroyed = 0;
struct ConditionTemporary {
    bool value;
    explicit ConditionTemporary(bool value) : value(value) { ++condition_alive; }
    ConditionTemporary(const ConditionTemporary&) = delete;
    ConditionTemporary(ConditionTemporary&&) = delete;
    ~ConditionTemporary() { --condition_alive; ++condition_destroyed; }
    bool read() const & { assert(condition_alive == 1); return value; }
};
[[clang::annotate("suspend")]]
void* condition_wait(std::shared_ptr<Continuation<void*>> completion) {
    void* raw = await_value(completion);
    std::unique_ptr<int> box(static_cast<int*>(raw));
    assert(condition_alive == 1 && condition_destroyed == 0 && *box == 7);
    return nullptr;
}
const int* borrowed_integer = nullptr;
bool read_integer(const int& value) { borrowed_integer = std::addressof(value); return value == 17; }
[[clang::annotate("suspend")]]
void* scalar_condition_wait(std::shared_ptr<Continuation<void*>> completion) {
    void* raw = await_value(completion);
    std::unique_ptr<int> box(static_cast<int*>(raw));
    assert(borrowed_integer && *borrowed_integer == 17 && *box == 7);
    return nullptr;
}
[[clang::annotate("suspend")]]
void* condition_flow(int operation, std::shared_ptr<Continuation<void*>> completion) {
    bool selected = false;
    if (operation == 0) selected = ConditionTemporary(false).read() && condition_wait(completion) == nullptr;
    if (operation == 1) selected = ConditionTemporary(true).read() || condition_wait(completion) == nullptr;
    if (operation == 2) selected = ConditionTemporary(true).read() && condition_wait(completion) == nullptr;
    if (operation == 3) selected = ConditionTemporary(false).read() || condition_wait(completion) == nullptr;
    if (operation == 4) selected = ConditionTemporary(true).read() ? condition_wait(completion) == nullptr : false;
    if (operation == 5) selected = ConditionTemporary(false).read() ? false : condition_wait(completion) == nullptr;
    if (operation == 6) selected = read_integer(17) && scalar_condition_wait(completion) == nullptr;
    assert(condition_alive == 0 && condition_destroyed == (operation == 6 ? 0 : 1));
    return new int(selected ? 1 : 0);
}
int main() {
    for (mode = 0; mode < 6; ++mode) {
        calls = 0;
        binding_evaluations = 0;
        array_copies = array_destructions = 0;
        array_sources = 0;
        branch_constructed = branch_destroyed = 0;
        empty_branch_inits = 0;
        auto done = std::make_shared<Done>();
        assert(ordinary_type_queries(done) && calls == 0);
        try {
            void* result = qualified_locals(37 + mode, done);
            if (intrinsics::is_coroutine_suspended(result)) {
                while (pending) {
                    assert(alive == 4 && array_alive == 4 && branch_alive == 1 && done->resumes == 0);
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("resumed failure"))));
                    else if (mode == 3 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(CancellationException("cancelled"))));
                    else held->resume_with(Result<void*>::success(new int(7)));
                }
            } else done->resume_with(Result<void*>::success(result));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->resumes == 1 && done->failed == (mode >= 2));
        assert(done->cancelled == (mode == 3));
        assert(done->failed || done->value == 542);
        assert(calls == (mode == 5 ? 0 : mode == 4 ? 1 : 2));
        assert(alive == 0 && !pending && frame.expired());
        assert(branch_alive == 0 && branch_constructed == calls && branch_destroyed == calls);
        assert(empty_branch_inits == calls);
        assert(array_alive == 0 && array_copies == 2 && array_sources == 1);
        assert(array_destructions == (mode == 5 ? 3 : 4));
        assert(array_destroyed[0] == (mode == 5 ? 11 : 12));
        assert(array_destroyed[1] == (mode == 5 ? 12 : 11));
        assert(array_destroyed[2] == (mode == 5 ? 11 : 12));
        if (mode != 5) assert(array_destroyed[3] == 99);
        assert(*started_identity == mode + 1);
        assert(*finished_identity == (mode == 0 ? 1 : 2));
    }
    for (int operation = 0; operation != 7; ++operation) for (mode = 0; mode != 5; ++mode) {
        calls = condition_destroyed = 0;
        auto done = std::make_shared<Done>();
        try {
            auto result = condition_flow(operation, done);
            if (intrinsics::is_coroutine_suspended(result)) {
                assert(condition_alive == (operation == 6 ? 0 : 1) && condition_destroyed == 0 && pending);
                auto held = std::move(pending);
                if (mode == 2) held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("condition"))));
                else if (mode == 3) held->resume_with(Result<void*>::failure(std::make_exception_ptr(CancellationException("condition"))));
                else held->resume_with(Result<void*>::success(new int(7)));
            } else done->resume_with(Result<void*>::success(result));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->resumes == 1 && done->failed == (operation >= 2 && mode >= 2));
        assert(done->cancelled == (operation >= 2 && mode == 3));
        assert(done->failed || done->value == (operation == 0 ? 0 : 1));
        assert(calls == (operation >= 2 ? 1 : 0));
        assert(condition_alive == 0 && condition_destroyed == (operation == 6 ? 0 : 1) && !pending && frame.expired());
    }
}
