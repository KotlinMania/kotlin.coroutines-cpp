// NOTE(port): Compiler regression for immutable reads, mutable snapshots,
// observable loads and side-effect order around a nested suspension.
#include "kotlinx/coroutines/dsl/Coroutines.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include <cassert>
#include <iostream>
#include <vector>
using namespace kotlinx::coroutines;
std::shared_ptr<Continuation<void*>> pending;
const volatile int volatile_value = 7;
int effects = 0;
int change(int& value) { value = 99; ++effects; return 3; }
[[clang::annotate("suspend")]] void* external_call(int mode, std::shared_ptr<Continuation<void*>> caller) {
    if (mode == 4) throw std::runtime_error("immediate-argument");
    if (!mode) return new int(41);
    pending = std::move(caller);
    return intrinsics::get_COROUTINE_SUSPENDED();
}
void* combine(int literal, int fixed, int snapshot, int changed, int observed, void* raw) {
    std::unique_ptr<int> result(static_cast<int*>(raw));
    return new int(literal + fixed + snapshot + changed + observed + *result);
}
[[suspend]] void* slicing(int mode, std::shared_ptr<Continuation<void*>> caller) {
    const int fixed = 40;
    int value = 2;
    return combine(1, fixed, value, change(value), volatile_value, external_call(mode, caller));
}
std::vector<int> tail_order;
int tail_effect(int value) { tail_order.push_back(value); return value; }
void* combine_tail(void* raw, int first, int second) {
    std::unique_ptr<int> result(static_cast<int*>(raw));
    return new int(*result + first + second);
}
[[suspend]] void* trailing(int mode, std::shared_ptr<Continuation<void*>> caller) {
    return combine_tail(external_call(mode, caller), tail_effect(1), tail_effect(2));
}
struct Done final : Continuation<void*> {
    int calls = 0;
    int value = 0;
    bool failed = false;
    bool cancelled = false;
    std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
    void resume_with(Result<void*> result) override {
        ++calls;
        try {
            std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
            value = *box;
        } catch (const CancellationException&) { failed = true; cancelled = true; }
        catch (const std::runtime_error&) { failed = true; }
    }
};
int constructions = 0;
int argument_alive = 0;
int argument_destroyed = 0;
std::vector<int> argument_destroy_order;
struct TemporaryArgument {
    int value;
    explicit TemporaryArgument(int value) : value(value) { ++argument_alive; }
    TemporaryArgument(const TemporaryArgument&) = delete;
    TemporaryArgument(TemporaryArgument&&) = delete;
    ~TemporaryArgument() {
        --argument_alive;
        ++argument_destroyed;
        argument_destroy_order.push_back(value);
    }
};
[[clang::annotate("suspend")]]
void* borrowing_argument(const TemporaryArgument& value, int mode,
                        std::shared_ptr<Continuation<void*>> caller) {
    const auto* identity = std::addressof(value);
    int total = value.value;
    for (int index = 0; index != 2; ++index) {
        void* raw = external_call(mode, caller);
        std::unique_ptr<int> box(static_cast<int*>(raw));
        assert(argument_alive == 2 && std::addressof(value) == identity && value.value == 2);
        total += *box;
    }
    return new int(total);
}
void* finish_argument(const TemporaryArgument& value, void* raw) {
    std::unique_ptr<int> box(static_cast<int*>(raw));
    assert(argument_alive == 2 && value.value == 1);
    return new int(*box + value.value);
}
[[clang::annotate("suspend")]]
void* owning_arguments(int mode, std::shared_ptr<Continuation<void*>> caller) {
    void* raw = finish_argument(TemporaryArgument(1),
        borrowing_argument(TemporaryArgument(2), mode, caller));
    assert(argument_alive == 0 && argument_destroyed == 2);
    return raw;
}
struct Constructed {
    int value;
    Constructed(int snapshot, int changed, void* raw) {
        std::unique_ptr<int> box(static_cast<int*>(raw));
        value = snapshot + changed + *box;
        ++constructions;
    }
};
[[suspend]] void* constructing(int mode, std::shared_ptr<Continuation<void*>> caller) {
    int value = 2;
    Constructed item(value, change(value), external_call(mode, caller));
    return new int(item.value);
}
void* constructed_result(const Constructed& item) { return new int(item.value); }
[[suspend]] void* constructing_expression(int mode, std::shared_ptr<Continuation<void*>> caller) {
    int value = 2;
    return constructed_result(Constructed(value, change(value), external_call(mode, caller)));
}
struct DefaultConstructed {
    int value;
    DefaultConstructed(void* raw, int first = tail_effect(1), int second = tail_effect(2)) {
        std::unique_ptr<int> box(static_cast<int*>(raw));
        value = *box + first + second;
        ++constructions;
    }
};
void* default_constructed_result(const DefaultConstructed& item) { return new int(item.value); }
[[suspend]] void* default_constructing(int mode, bool expression, std::shared_ptr<Continuation<void*>> caller) {
    if (expression) return default_constructed_result(DefaultConstructed(external_call(mode, caller)));
    DefaultConstructed item(external_call(mode, caller));
    return new int(item.value);
}
int main() {
    for (int mode = 0; mode != 5; ++mode) {
        argument_destroyed = 0;
        argument_destroy_order.clear();
        auto done = std::make_shared<Done>();
        void* result = nullptr;
        try { result = owning_arguments(mode, done); }
        catch (const std::runtime_error&) {
            assert(mode == 4);
            done->resume_with(Result<void*>::failure(std::current_exception()));
        }
        if (mode == 4) {
            assert(done->calls == 1 && done->failed && !done->cancelled);
        } else if (!mode) {
            std::unique_ptr<int> box(static_cast<int*>(result));
            assert(*box == 85 && done->calls == 0);
        } else {
            assert(intrinsics::is_coroutine_suspended(result));
            assert(argument_alive == 2 && argument_destroyed == 0);
            for (int index = 0; pending; ++index) {
                assert(index < 2 && argument_alive == 2 && argument_destroyed == 0);
                auto held = std::move(pending);
                if (mode == 2) held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("borrowed-argument"))));
                else if (mode == 3 && index == 1) held->resume_with(Result<void*>::failure(
                    std::make_exception_ptr(CancellationException("borrowed-argument"))));
                else held->resume_with(Result<void*>::success(new int(41)));
                held.reset();
                if (pending) assert(done->calls == 0 && argument_alive == 2 && argument_destroyed == 0);
            }
            assert(done->calls == 1 && done->failed == (mode >= 2));
            assert(done->cancelled == (mode == 3));
            if (mode == 1) assert(done->value == 85);
        }
        assert(argument_alive == 0 && argument_destroyed == 2 && !pending);
        assert(argument_destroy_order == (std::vector<int>{2, 1}));
    }
    for (bool expression : {false, true}) {
        for (int mode = 0; mode != 3; ++mode) {
            tail_order.clear();
            constructions = 0;
            auto done = std::make_shared<Done>();
            auto result = default_constructing(mode, expression, done);
            if (!mode) {
                std::unique_ptr<int> box(static_cast<int*>(result));
                assert(*box == 44 && constructions == 1 && tail_order == (std::vector<int>{1, 2}));
            } else {
                assert(intrinsics::is_coroutine_suspended(result) && constructions == 0 && tail_order.empty());
                auto held = std::move(pending);
                if (mode == 1) held->resume_with(Result<void*>::success(new int(41)));
                else held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("constructor-default"))));
                held.reset();
                assert(done->calls == 1 && done->failed == (mode == 2));
                if (mode == 1) assert(done->value == 44 && constructions == 1 && tail_order == (std::vector<int>{1, 2}));
                else assert(constructions == 0 && tail_order.empty());
            }
        }
    }
    for (int mode = 0; mode != 3; ++mode) {
        effects = constructions = 0;
        auto done = std::make_shared<Done>();
        auto result = constructing_expression(mode, done);
        assert(effects == 1);
        if (!mode) {
            std::unique_ptr<int> box(static_cast<int*>(result));
            assert(*box == 46 && constructions == 1 && done->calls == 0);
        } else {
            assert(intrinsics::is_coroutine_suspended(result) && constructions == 0 && pending);
            auto held = std::move(pending);
            if (mode == 1) held->resume_with(Result<void*>::success(new int(41)));
            else held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("constructor-expression"))));
            held.reset();
            assert(done->calls == 1 && done->failed == (mode == 2) && effects == 1);
            assert(constructions == (mode == 1 ? 1 : 0));
            if (mode == 1) assert(done->value == 46);
        }
    }
    for (int mode = 0; mode != 3; ++mode) {
        effects = constructions = 0;
        auto done = std::make_shared<Done>();
        auto result = constructing(mode, done);
        assert(effects == 1);
        if (!mode) {
            std::unique_ptr<int> box(static_cast<int*>(result));
            assert(*box == 46 && constructions == 1 && done->calls == 0);
        } else {
            assert(intrinsics::is_coroutine_suspended(result) && constructions == 0 && pending);
            auto held = std::move(pending);
            if (mode == 1) held->resume_with(Result<void*>::success(new int(41)));
            else held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("constructor"))));
            held.reset();
            assert(done->calls == 1 && done->failed == (mode == 2) && effects == 1);
            assert(constructions == (mode == 1 ? 1 : 0));
            if (mode == 1) assert(done->value == 46);
        }
    }
    for (int mode = 0; mode != 3; ++mode) {
        effects = 0;
        auto done = std::make_shared<Done>();
        auto result = slicing(mode, done);
        assert(effects == 1);
        if (!mode) {
            std::unique_ptr<int> box(static_cast<int*>(result));
            assert(*box == 94 && done->calls == 0 && !pending);
        } else {
            assert(intrinsics::is_coroutine_suspended(result) && pending && done->calls == 0);
            auto held = std::move(pending);
            if (mode == 1) held->resume_with(Result<void*>::success(new int(41)));
            else held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("nested"))));
            held.reset();
            assert(done->calls == 1 && done->failed == (mode == 2) && effects == 1 && !pending);
            if (mode == 1) assert(done->value == 94);
        }
    }
    for (int mode = 0; mode != 3; ++mode) {
        tail_order.clear();
        auto done = std::make_shared<Done>();
        auto result = trailing(mode, done);
        if (!mode) {
            std::unique_ptr<int> box(static_cast<int*>(result));
            assert(*box == 44 && done->calls == 0 && tail_order == (std::vector<int>{1, 2}));
        } else {
            assert(intrinsics::is_coroutine_suspended(result) && pending && tail_order.empty());
            auto held = std::move(pending);
            if (mode == 1) held->resume_with(Result<void*>::success(new int(41)));
            else held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("tail"))));
            held.reset();
            assert(done->calls == 1 && done->failed == (mode == 2) && !pending);
            if (mode == 1) assert(done->value == 44 && tail_order == (std::vector<int>{1, 2}));
            else assert(tail_order.empty());
        }
    }
    std::cout << "slicing:94; immutable reads; mutable snapshot; volatile load; effects once; resumed failure; trailing effects:44 in order\n";
}
