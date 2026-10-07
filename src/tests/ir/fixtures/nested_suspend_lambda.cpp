// NOTE(port): Compiler execution regression for nested callable construction,
// capture identity, retained lifetime and immediate/resumed failure propagation.
#include "kotlinx/coroutines/dsl/Coroutines.hpp"
#include <cassert>
#include <iostream>
using namespace kotlinx::coroutines;
std::shared_ptr<Continuation<void*>> pending;
int external_calls = 0;
int destroyed = 0;
struct Tracked {
    ~Tracked() { ++destroyed; }
};
[[clang::annotate("suspend")]] void* external_call(int mode, std::shared_ptr<Continuation<void*>> caller) {
    ++external_calls;
    if (!mode) return new int(41);
    pending = std::move(caller);
    return intrinsics::get_COROUTINE_SUSPENDED();
}
struct Done final : Continuation<void*> {
    int calls = 0;
    int value = 0;
    bool failed = false;
    std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
    void resume_with(Result<void*> result) override {
        ++calls;
        try {
            std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
            value = *box;
        } catch (const std::runtime_error&) { failed = true; }
    }
};
[[suspend]] void* entry(int bias, int mode, std::shared_ptr<Continuation<void*>> caller) {
    int visits = 0;
    auto block = [bias, &visits, owned = std::make_unique<Tracked>()] [[suspend]]
        (int mode, std::shared_ptr<Continuation<void*>> caller) -> void* {
        void* raw = external_call(mode, caller);
        std::unique_ptr<int> result(static_cast<int*>(raw));
        ++visits;
        assert(owned);
        return new int(*result + bias + visits);
    };
    bias = 100;
    visits = 2;
    void* raw = external_call(mode, caller);
    std::unique_ptr<int> result(static_cast<int*>(raw));
    assert(*result == 41);
    return block(mode, caller);
}
struct Owner {
    int bias = 1;
    [[suspend]] void* copied_const_entry(int mode, std::shared_ptr<Continuation<void*>> caller) const {
        auto block = [*this] [[suspend]] (int mode, std::shared_ptr<Continuation<void*>> caller) -> void* {
            void* raw = external_call(mode, caller);
            std::unique_ptr<int> result(static_cast<int*>(raw));
            return new int(*result + bias);
        };
        void* raw = external_call(mode, caller);
        std::unique_ptr<int> result(static_cast<int*>(raw));
        return block(mode, caller);
    }
    [[suspend]] void* copied_entry(int mode, std::shared_ptr<Continuation<void*>> caller) {
        auto block = [*this] [[suspend]] (int mode, std::shared_ptr<Continuation<void*>> caller) mutable -> void* {
            void* raw = external_call(mode, caller);
            std::unique_ptr<int> result(static_cast<int*>(raw));
            ++bias;
            return new int(*result + bias);
        };
        void* raw = external_call(mode, caller);
        std::unique_ptr<int> result(static_cast<int*>(raw));
        return block(mode, caller);
    }
    [[suspend]] void* entry(int mode, std::shared_ptr<Continuation<void*>> caller) {
        auto block = [this] [[suspend]] (int mode, std::shared_ptr<Continuation<void*>> caller) -> void* {
            void* raw = external_call(mode, caller);
            std::unique_ptr<int> result(static_cast<int*>(raw));
            return new int(*result + bias);
        };
        void* raw = external_call(mode, caller);
        std::unique_ptr<int> result(static_cast<int*>(raw));
        return block(mode, caller);
    }
};
[[suspend]] void* collisions(int f_0_, std::shared_ptr<Continuation<void*>> caller) {
    auto block = [f_0_, count = 3] [[suspend]] (int f_1_, std::shared_ptr<Continuation<void*>> caller) mutable -> void* {
        void* raw = external_call(1, caller);
        std::unique_ptr<int> result(static_cast<int*>(raw));
        int sum = f_0_, shadow = f_1_;
        { int f_0_ = 100; sum += f_0_; }
        f_0_ += 2;
        return new int(*result + sum + f_0_ + f_1_ + count + shadow);
    };
    void* raw = external_call(1, caller);
    std::unique_ptr<int> result(static_cast<int*>(raw));
    return block(5, caller);
}
[[suspend]] void* array_captures(std::shared_ptr<Continuation<void*>> caller) {
    int values[2][2] = {{1, 2}, {3, 4}};
    int borrowed[2] = {5, 6};
    auto block = [values, &borrowed] [[suspend]] (std::shared_ptr<Continuation<void*>> caller) mutable -> void* {
        void* raw = external_call(1, caller);
        std::unique_ptr<int> result(static_cast<int*>(raw));
        values[0][0] += 10;
        assert(sizeof(values) == sizeof(int) * 4);
        return new int(*result + values[0][0] + values[1][1] + borrowed[0]);
    };
    values[0][0] = 100;
    borrowed[0] = 7;
    void* raw = external_call(1, caller);
    std::unique_ptr<int> result(static_cast<int*>(raw));
    return block(caller);
}
int array_items_live = 0;
struct ArrayItem {
    int value;
    explicit ArrayItem(int value) : value(value) { ++array_items_live; }
    ArrayItem(const ArrayItem& source) : value(source.value) { ++array_items_live; }
    ~ArrayItem() { --array_items_live; }
};
[[suspend]] void* object_array_captures(std::shared_ptr<Continuation<void*>> caller) {
    ArrayItem items[2] = {ArrayItem(1), ArrayItem(2)};
    auto block = [items] [[suspend]] (std::shared_ptr<Continuation<void*>> caller) -> void* {
        void* raw = external_call(1, caller);
        std::unique_ptr<int> result(static_cast<int*>(raw));
        return new int(*result + items[0].value + items[1].value);
    };
    items[0].value = 100;
    void* raw = external_call(1, caller);
    std::unique_ptr<int> result(static_cast<int*>(raw));
    return block(caller);
}
int main() {
    auto immediate_done = std::make_shared<Done>();
    std::unique_ptr<int> immediate(static_cast<int*>(entry(-1, 0, immediate_done)));
    assert(*immediate == 43 && immediate_done->calls == 0 && destroyed == 1 && external_calls == 2);
    for (int failure = 0; failure != 3; ++failure) {
        auto done = std::make_shared<Done>();
        assert(intrinsics::is_coroutine_suspended(entry(-1, 1, done)));
        auto first = std::move(pending);
        std::weak_ptr<Continuation<void*>> outer = first;
        if (failure == 1) {
            first->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("outer"))));
            assert(!pending && done->calls == 1 && done->failed);
        } else {
            first->resume_with(Result<void*>::success(new int(41)));
            first.reset();
            assert(pending && done->calls == 0 && !outer.expired());
            auto second = std::move(pending);
            if (failure == 2)
                second->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("inner"))));
            else second->resume_with(Result<void*>::success(new int(41)));
            assert(done->calls == 1 && done->failed == (failure == 2));
            if (!failure) assert(done->value == 43);
            second.reset();
        }
        first.reset();
        assert(outer.expired() && destroyed == failure + 2);
    }
    assert(external_calls == 7 && !pending);
    Owner owner;
    auto member_done = std::make_shared<Done>();
    assert(intrinsics::is_coroutine_suspended(owner.entry(1, member_done)));
    auto first = std::move(pending);
    first->resume_with(Result<void*>::success(new int(41)));
    first.reset();
    assert(pending && member_done->calls == 0);
    owner.bias = 41;
    auto second = std::move(pending);
    second->resume_with(Result<void*>::success(new int(41)));
    second.reset();
    assert(member_done->calls == 1 && member_done->value == 82 && !pending && destroyed == 4);
    auto collision_done = std::make_shared<Done>();
    assert(intrinsics::is_coroutine_suspended(collisions(7, collision_done)));
    first = std::move(pending);
    first->resume_with(Result<void*>::success(new int(41)));
    first.reset();
    assert(pending && collision_done->calls == 0);
    second = std::move(pending);
    second->resume_with(Result<void*>::success(new int(41)));
    second.reset();
    assert(collision_done->calls == 1 && collision_done->value == 170 && !pending && destroyed == 4);
    auto copied_done = std::make_shared<Done>();
    owner.bias = 1;
    assert(intrinsics::is_coroutine_suspended(owner.copied_entry(1, copied_done)));
    owner.bias = 100;
    first = std::move(pending);
    first->resume_with(Result<void*>::success(new int(41)));
    first.reset();
    assert(pending && copied_done->calls == 0);
    owner.bias = 200;
    second = std::move(pending);
    second->resume_with(Result<void*>::success(new int(41)));
    second.reset();
    assert(copied_done->calls == 1 && copied_done->value == 43 && owner.bias == 200 && !pending);
    auto copied_const_done = std::make_shared<Done>();
    owner.bias = 1;
    assert(intrinsics::is_coroutine_suspended(owner.copied_const_entry(1, copied_const_done)));
    owner.bias = 100;
    first = std::move(pending);
    first->resume_with(Result<void*>::success(new int(41)));
    first.reset();
    assert(pending && copied_const_done->calls == 0);
    second = std::move(pending);
    second->resume_with(Result<void*>::success(new int(41)));
    second.reset();
    assert(copied_const_done->calls == 1 && copied_const_done->value == 42 && owner.bias == 100 && !pending);
    auto array_done = std::make_shared<Done>();
    assert(intrinsics::is_coroutine_suspended(array_captures(array_done)));
    first = std::move(pending);
    first->resume_with(Result<void*>::success(new int(41)));
    first.reset();
    assert(pending && array_done->calls == 0);
    second = std::move(pending);
    second->resume_with(Result<void*>::success(new int(41)));
    second.reset();
    assert(array_done->calls == 1 && array_done->value == 63 && !pending);
    for (bool failure : {false, true}) {
        auto object_done = std::make_shared<Done>();
        assert(intrinsics::is_coroutine_suspended(object_array_captures(object_done)));
        assert(array_items_live >= 4);
        first = std::move(pending);
        first->resume_with(Result<void*>::success(new int(41)));
        first.reset();
        assert(pending && object_done->calls == 0 && array_items_live >= 4);
        second = std::move(pending);
        if (failure) second->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("array"))));
        else second->resume_with(Result<void*>::success(new int(41)));
        second.reset();
        assert(object_done->calls == 1 && object_done->failed == failure && array_items_live == 0 && !pending);
        if (!failure) assert(object_done->value == 44);
    }
    std::cout << "nested lambda:43; outer failure; inner failure; captures released:4; captured this:82; shadowed captures:170; copied this:43; copied const this:42; arrays:63; object arrays:44; array cleanup\n";
}
