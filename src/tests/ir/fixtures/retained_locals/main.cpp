#include "api.hpp"
#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <deque>
#include <kotlinx/coroutines/CoroutineDispatcher.hpp>
#include <kotlinx/coroutines/JobImpl.hpp>
int Tracked::alive = 0;
int Tracked::throw_value = -1;
int CallToken::alive = 0;
int CatchValue::alive = 0;
int CatchValue::copies = 0;
int Range::alive = 0;
int Condition::alive = 0;
int sequence = 0;
bool ordered = false;
bool assignment_order = false;
bool dependent_order = false;
bool fail_second = false;
void sequence_event(int digit) { sequence = sequence * 10 + digit; }
struct Done : Continuation<void*> {
    int calls=0, value=0; bool failed=false;
    std::exception_ptr failure;
    std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance();
    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> result) override {
        ++calls;
        try { auto boxed=std::unique_ptr<int>(static_cast<int*>(result.get_or_throw())); value=*boxed; }
        catch(const CancellationException&) {failed=true; failure=std::current_exception();}
        catch(const std::runtime_error&) {failed=true; failure=std::current_exception();}
    }
};
struct YieldDispatcher : CoroutineDispatcher {
    mutable std::deque<std::shared_ptr<Runnable>> queue;
    mutable int yield_calls = 0;
    void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> block) const override {
        queue.push_back(std::move(block));
    }
    void dispatch_yield(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
        ++yield_calls;
        CoroutineDispatcher::dispatch_yield(context, std::move(block));
    }
    void run_next() {
        auto task = std::move(queue.front());
        queue.pop_front();
        task->run();
    }
};
struct TimerDispatcher final : YieldDispatcher, Delay {
    std::shared_ptr<CancellableContinuationImpl<void>> timer;
    long long scheduled_millis = 0;
    int schedules = 0;
    void schedule_resume_after_delay(long long time_millis, CancellableContinuation<void>& continuation) override {
        assert(!timer);
        ++schedules;
        scheduled_millis = time_millis;
        timer = dynamic_cast<CancellableContinuationImpl<void>&>(continuation).shared_from_this();
    }
    void fire() {
        auto held = std::move(timer);
        held->resume(nullptr);
    }
};
int mode=0, next_value=0, calls=0;
std::shared_ptr<Continuation<void*>> pending;
std::weak_ptr<Continuation<void*>> frame;
void* external_call(int value, CallToken token, std::shared_ptr<Continuation<void*>> completion) {
    assert(token.value == value && CallToken::alive == 1);
    ++calls; frame=completion; next_value=value+1;
    if (ordered) {
        assert(sequence == (calls <= 2 ? 1 : calls == 3 ? 12 : 123));
        assert(Tracked::alive == (calls == 1 || calls >= 4 ? 1 : 0));
    }
    if (assignment_order) assert(sequence == (calls == 1 ? 0 : calls == 2 ? 1 : calls == 3 ? 12 : 123));
    if(dependent_order) assert(sequence == (calls == 1 ? 1 : 12));
    if(mode==1 || mode==2) {pending=completion; return intrinsics::get_COROUTINE_SUSPENDED();}
    if(mode==3 && (!fail_second || calls == 2)) throw std::runtime_error("immediate failure");
    return new int(next_value);

}
int* expected_indirect_reference = nullptr;
int reference_indirect_target(int&& value, int result) {
    assert(std::addressof(value) == expected_indirect_reference);
    return ++value + result;
}
int temporary_indirect_target(const int& value, int result) { return value + 1 + result; }
int original_indirect_target(int value) { return value + 100; }
int changed_indirect_target(int value) { return value + 1000; }
int main(int argc, char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "noexcept-terminate") {
        std::set_terminate([] { std::_Exit(calls == 1 ? 86 : 89); });
        mode = 3;
        authoring::nested::UnknownSuspendReceiver receiver;
        const authoring::nested::NestedReceiver<authoring::nested::UnknownSuspendReceiver>::Entry nested(receiver);
        try { nested.checked<true>(std::make_shared<Done>()); }
        catch (...) { return 88; }
        return 87;
    }
    for (mode = 0; mode < 4; ++mode) {
        sequence = 0;
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_field_assignment(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(sequence == (calls == 1 ? 1 : 12));
                    assert(Tracked::alive == (calls == 1 ? 1 : 2));
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("field value failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 83);
        assert(calls == (mode == 3 ? 1 : 2));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for(mode=0; mode<5; ++mode) {
        calls=0;
        Tracked::throw_value = mode == 4 ? 42 : -1;
        auto done=std::make_shared<Done>();
        try {
            auto outcome=retained_value(41,done);
            if(intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive==4 && CallToken::alive==0 && done->calls==0 && pending!=done);
                while(pending) {
                    assert(CallToken::alive == 0);
                    auto held=std::move(pending);
                    if(mode==2) held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("resume failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch(...) {done->resume_with(Result<void*>::failure(std::current_exception()));}
        assert(done->calls==1 && done->failed==(mode>=2));
        assert(done->failed || done->value==530);
        assert(calls==(mode==4 ? 0 : mode>=2 ? 1 : 3));
        assert(Tracked::alive==0 && CallToken::alive==0 && frame.expired() && !pending);
        std::cout << "retained:" << mode << ":" << (done->failed ? "failure" : "530") << "\n";
    }
    Tracked::throw_value = -1;
    for (mode = 0; mode < 2; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        auto outcome = retained_condition(done);
        if (intrinsics::is_coroutine_suspended(outcome)) {
            while (pending) {
                assert(Tracked::alive == 0 && CallToken::alive == 0);
                auto held = std::move(pending);
                held->resume_with(Result<void*>::success(new int(next_value)));
            }
        } else done->resume_with(Result<void*>::success(outcome));
        assert(done->calls == 1 && !done->failed && done->value == 3 && calls == 3);
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_switch(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    bool condition = calls == 1 || calls == 3 || calls == 5 || calls == 8;
                    assert(Tracked::alive == (condition ? 2 : 3) && CallToken::alive == 0);
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("case failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 43);
        assert(calls == (mode == 2 ? 2 : mode == 3 ? 1 : 9));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_range(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(Tracked::alive == 1 && Range::alive == (calls > 3 ? 1 : 0));
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 5)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("range failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 27);
        assert(calls == (mode == 2 ? 5 : mode == 3 ? 1 : 6));
        assert(frame.expired() && Range::alive == 0 && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_condition_scope(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    bool increment = calls == 6 || calls == 8 || calls == 10;
                    assert(Condition::alive == (increment ? 1 : 0) && CallToken::alive == 0);
                    assert(Tracked::alive == (calls <= 2 || (calls >= 5 && calls <= 11) ? 1 : 0));
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 8)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("condition failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 25);
        assert(calls == (mode == 2 ? 8 : mode == 3 ? 1 : 14));
        assert(frame.expired() && Condition::alive == 0 && Tracked::alive == 0 && CallToken::alive == 0);
    }
    ordered = true;
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        sequence = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_comma(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(sequence == (calls <= 2 ? 1 : calls == 3 ? 12 : 123));
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("comma failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 45);
        assert(calls == (mode == 2 ? 2 : mode == 3 ? 1 : 6));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    ordered = false;
    assignment_order = true;
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        sequence = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_assignment(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(sequence == (calls == 1 ? 0 : calls == 2 ? 1 : calls == 3 ? 12 : 123));
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 4)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("destination failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 50);
        assert(calls == (mode == 2 ? 4 : mode == 3 ? 1 : 5));
        assert(frame.expired() && CallToken::alive == 0);
    }
    assignment_order = false;
    for (int lookup = 0; lookup < 8; ++lookup) for (bool second_failure : {false, true}) for (bool suspended_overload : {false, true}) for (mode = 0; mode < 4; ++mode) {
        fail_second = second_failure; calls = 0; frame.reset();
        auto done = std::make_shared<Done>();
        authoring::nested::UnknownSuspendReceiver suspend_receiver;
        authoring::nested::UnknownOrdinaryReceiver ordinary_receiver;
        const authoring::nested::LateReceiver late_receiver;
        const authoring::nested::ExplicitOuter<int>::Entry<int> explicit_entry;
        const authoring::nested::LateClassReceiver suspend_class(suspend_receiver);
        const authoring::nested::LateClassReceiver ordinary_class(ordinary_receiver);
        const authoring::nested::NestedReceiver<authoring::nested::UnknownSuspendReceiver>::Entry suspend_nested(suspend_receiver);
        const authoring::nested::NestedReceiver<authoring::nested::UnknownOrdinaryReceiver>::Entry ordinary_nested(ordinary_receiver);
        try {
            auto outcome = lookup == 7 ?
                (suspended_overload ? explicit_entry.run(suspend_receiver, done) : explicit_entry.run(ordinary_receiver, done)) : lookup == 6 ?
                (suspended_overload ? (second_failure || mode == 3 ? suspend_nested.checked<false>(done) : suspend_nested.checked<true>(done)) :
                    (second_failure || mode == 3 ? ordinary_nested.checked<false>(done) : ordinary_nested.checked<true>(done))) : lookup == 5 ?
                (suspended_overload ? suspend_nested.run(done) : ordinary_nested.run(done)) : lookup == 4 ?
                (suspended_overload ? suspend_class.run(done) : ordinary_class.run(done)) : lookup == 3 ?
                (suspended_overload ? late_receiver.run(suspend_receiver, done) : late_receiver.run(ordinary_receiver, done)) : lookup == 2 ?
                (suspended_overload ? authoring::nested::retained_adl(authoring::nested::adl::SuspendArgument{}, done) :
                    authoring::nested::retained_adl(authoring::nested::adl::OrdinaryArgument{}, done)) : lookup == 1 ?
                (suspended_overload ? authoring::nested::retained_unknown_member(suspend_receiver, done) :
                    authoring::nested::retained_unknown_member(ordinary_receiver, done)) :
                suspended_overload ? authoring::nested::retained_partial_mixed(41, done) :
                authoring::nested::retained_partial_mixed(36.0, done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(Tracked::alive == 1);
                    auto held = std::move(pending);
                    if (mode == 2 && calls == (second_failure ? 2 : 1))
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("partial mixed failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2 && (!second_failure || suspended_overload)));
        assert(done->failed || done->value == (suspended_overload ? 48 : 78));
        assert(calls == (mode >= 2 && !second_failure ? 1 : suspended_overload ? 2 : 1));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    fail_second = false;
    for (bool cross_tu : {false, true}) for (bool alternate_constant : {false, true}) for (bool suspended_overload : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0; frame.reset();
        auto done = std::make_shared<Done>();
        try {
            auto outcome = cross_tu ?
                (alternate_constant ?
                    (suspended_overload ? source_mixed_seven(41, done) : source_mixed_seven(36.0, done)) :
                    (suspended_overload ? source_mixed_five(41, done) : source_mixed_five(36.0, done))) :
                (alternate_constant ?
                    (suspended_overload ? authoring::nested::retained_mixed<int, 7>(41, done) : authoring::nested::retained_mixed<double, 7>(36.0, done)) :
                    (suspended_overload ? authoring::nested::retained_mixed(41, done) : authoring::nested::retained_mixed(36.0, done)));
            if (intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive == 1);
                auto held = std::move(pending);
                if (mode == 2) held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("mixed overload failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (suspended_overload && mode >= 2));
        if (!done->failed && done->value != (suspended_overload ? 42 : 72) + (alternate_constant ? 7 : 5))
            std::cerr << "mixed overload mismatch: alternate=" << alternate_constant << " suspend=" << suspended_overload
                      << " mode=" << mode << " value=" << done->value << std::endl;
        assert(done->failed || done->value == (suspended_overload ? 42 : 72) + (alternate_constant ? 7 : 5));
        assert(calls == (suspended_overload ? 1 : 0) && frame.expired());
        assert(Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (bool class_target : {false, true}) for (bool reference_target : {false, true}) for (bool suspended_target : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0; frame.reset();
        auto done = std::make_shared<Done>();
        try {
            authoring::nested::RetainedFunctionArgument<&authoring::nested::targets::suspend_target> class_pointer_suspend;
            authoring::nested::RetainedFunctionArgument<&authoring::nested::targets::ordinary_target> class_pointer_ordinary;
            authoring::nested::RetainedFunctionReference<authoring::nested::targets::suspend_target> class_reference_suspend;
            authoring::nested::RetainedFunctionReference<authoring::nested::targets::ordinary_target> class_reference_ordinary;
            auto outcome = class_target ?
                (reference_target ?
                    (suspended_target ? class_reference_suspend.run(41, done) : class_reference_ordinary.run(71, done)) :
                    (suspended_target ? class_pointer_suspend.run(41, done) : class_pointer_ordinary.run(71, done))) : reference_target ?
                (suspended_target ? authoring::nested::retained_function_reference<authoring::nested::targets::suspend_target>(41, done) :
                    authoring::nested::retained_function_reference<authoring::nested::targets::ordinary_target>(71, done)) : suspended_target ?
                authoring::nested::retained_function_argument<&authoring::nested::targets::suspend_target>(41, done) :
                authoring::nested::retained_function_argument<&authoring::nested::targets::ordinary_target>(71, done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive == 1);
                auto held = std::move(pending);
                if (mode == 2) held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("function argument failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (suspended_target && mode >= 2));
        assert(done->failed || done->value == (suspended_target ? 47 : 77));
        assert(calls == (suspended_target ? 1 : 0) && frame.expired());
        assert(Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (int argument = 0; argument < 3; ++argument) for (bool alternate : {false, true}) for (bool suspended_overload : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0; frame.reset();
        authoring::nested::values::five = 5;
        authoring::nested::values::seven = 7;
        int* expected = argument == 2 ? nullptr : alternate ? &authoring::nested::values::seven : &authoring::nested::values::five;
        int initial = expected ? *expected : 5;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = argument == 2 ?
                (suspended_overload ? authoring::nested::retained_pointer_argument<int, nullptr>(41, expected, done) :
                    authoring::nested::retained_pointer_argument<double, nullptr>(36.0, expected, done)) : argument == 1 ?
                (alternate ?
                    (suspended_overload ? authoring::nested::retained_reference_argument<int, authoring::nested::values::seven>(41, expected, done) : authoring::nested::retained_reference_argument<double, authoring::nested::values::seven>(36.0, expected, done)) :
                    (suspended_overload ? authoring::nested::retained_reference_argument<int, authoring::nested::values::five>(41, expected, done) : authoring::nested::retained_reference_argument<double, authoring::nested::values::five>(36.0, expected, done))) :
                (alternate ?
                    (suspended_overload ? authoring::nested::retained_pointer_argument<int, &authoring::nested::values::seven>(41, expected, done) : authoring::nested::retained_pointer_argument<double, &authoring::nested::values::seven>(36.0, expected, done)) :
                    (suspended_overload ? authoring::nested::retained_pointer_argument<int, &authoring::nested::values::five>(41, expected, done) : authoring::nested::retained_pointer_argument<double, &authoring::nested::values::five>(36.0, expected, done)));
            if (intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive == 1);
                if (expected) *expected += 2;
                auto held = std::move(pending);
                if (mode == 2) held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("symbol argument failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (suspended_overload && mode >= 2));
        assert(done->failed || done->value == (suspended_overload ? 42 : 72) + initial + (expected ? *expected : 5));
        assert(calls == (suspended_overload ? 1 : 0) && frame.expired());
        assert(Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (bool shared : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto unique = shared ? std::unique_ptr<Tracked>{} : std::make_unique<Tracked>(41);
        auto retained = shared ? std::make_shared<Tracked>(41) : std::shared_ptr<Tracked>{};
        auto done = std::make_shared<Done>();
        try {
            auto outcome = shared ? authoring::nested::retained_template(retained, done) :
                authoring::nested::retained_template(std::move(unique), done);
            assert(!unique);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive == 1 && CallToken::alive == 0);
                auto held = std::move(pending);
                if (mode == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("template failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 84);
        assert(calls == 1 && frame.expired() && Tracked::alive == (shared ? 1 : 0) && CallToken::alive == 0);
        assert(!shared || retained.use_count() == 1);
    }
    dependent_order = true;
    for (bool owned_receiver : {false, true}) for (fail_second = false; ; fail_second = true) {
        for (mode = 0; mode < 4; ++mode) {
            sequence = 0; calls = 0;
            auto receiver = owned_receiver ? std::unique_ptr<authoring::nested::DependentReceiver>{} :
                std::make_unique<authoring::nested::DependentReceiver>();
            auto done = std::make_shared<Done>();
            try {
                auto outcome = owned_receiver ? authoring::nested::retained_owned_dependent_member<authoring::nested::DependentReceiver>(done) :
                    authoring::nested::retained_dependent_member(*receiver, done);
                if (intrinsics::is_coroutine_suspended(outcome)) {
                    while (pending) {
                        assert(Tracked::alive == 2 && CallToken::alive == 0);
                        auto held = std::move(pending);
                        if (mode == 2 && calls == (fail_second ? 2 : 1))
                            held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("dependent member failure"))));
                        else held->resume_with(Result<void*>::success(new int(next_value)));
                    }
                } else done->resume_with(Result<void*>::success(outcome));
            } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
            assert(done->calls == 1 && done->failed == (mode >= 2));
            assert(done->failed || done->value == 89);
            assert(calls == (mode >= 2 && !fail_second ? 1 : 2));
            assert(frame.expired() && Tracked::alive == (owned_receiver ? 0 : 1) && CallToken::alive == 0);
        }
        if (fail_second) break;
    }
    dependent_order = false; fail_second = false;
    auto exercise_template_receiver = [&](auto ownership, bool generic_method) {
        authoring::nested::TemplateReceiver<decltype(ownership)> receiver(std::move(ownership));
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = generic_method ? receiver.run(std::make_unique<Tracked>(7), done) : receiver.run(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive == 2 && CallToken::alive == 0);
                auto held = std::move(pending);
                if (mode == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("member template failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == (generic_method ? 93 : 91));
        assert(calls == 1 && frame.expired() && Tracked::alive == 1 && CallToken::alive == 0);
    };
    for (bool shared : {false, true}) for (bool generic_method : {false, true}) for (mode = 0; mode < 4; ++mode) {
        if (shared) exercise_template_receiver(std::make_shared<Tracked>(41), generic_method);
        else exercise_template_receiver(std::make_unique<Tracked>(41), generic_method);
        assert(Tracked::alive == 0);
    }
    for (int choice = 0; choice < 3; ++choice) for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_nested_exit(choice, done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(Tracked::alive == 1 && CallToken::alive == 0);
                    auto held = std::move(pending);
                    if (mode == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("nested exit failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == (choice == 0 ? 77 : 88));
        assert(calls == (mode >= 2 || choice < 2 ? 1 : 2));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (bool nested : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = nested ? retained_nested_context(done) : retained_handler_loop(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(Tracked::alive == (nested && calls == 2 ? 3 : 2) && CallToken::alive == 0);
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("context failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == (nested ? 77 : 42));
        assert(calls == (mode == 3 ? 1 : mode == 2 ? 2 : nested ? 3 : 2));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (bool fail_handler : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        CatchValue::copies = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_value_catch(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(Tracked::alive == (calls == 1 ? 1 : 0));
                    assert(CatchValue::alive == (calls == 1 ? 0 : 2));
                    auto held = std::move(pending);
                    if (mode == 2 || (mode == 1 && fail_handler && calls == 2))
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("value catch failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2 || (mode == 1 && fail_handler)));
        assert(done->failed || done->value == 8);
        assert(calls == (mode >= 2 ? 1 : 2) && CatchValue::copies == (mode >= 2 ? 0 : 1));
        assert(frame.expired() && CatchValue::alive == 0 && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (bool fail_handler : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_exception(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(Tracked::alive == 2 && CallToken::alive == 0);
                    auto held = std::move(pending);
                    if (mode == 2 && (calls == 1 || fail_handler))
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("protected failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode == 3 || (mode == 2 && fail_handler)));
        assert(done->failed || done->value == (mode == 2 ? 48 : 42));
        assert(calls == (mode >= 2 ? 2 : 1));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        auto outcome = retained_rethrow(done);
        if (intrinsics::is_coroutine_suspended(outcome)) {
            while (pending) {
                assert(Tracked::alive == 2 && CallToken::alive == 0);
                auto held = std::move(pending);
                if (mode == 2 && calls == 1)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("rethrow failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            }
        } else done->resume_with(Result<void*>::success(outcome));
        assert(done->calls == 1 && !done->failed && done->value == (mode >= 2 ? 77 : 42));
        assert(calls == (mode >= 2 ? 2 : 1));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (int entry = 0; entry < 3; ++entry) for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        const Receiver receiver(41);
        auto done = std::make_shared<Done>();
        try {
            auto outcome = entry == 0 ? authoring::nested::header_value(done) :
                entry == 1 ? receiver.header_value(done) : source_header_value(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive == 2 && CallToken::alive == 0);
                auto held = std::move(pending);
                if (mode == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("header failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == (entry == 1 ? 95 : 47));
        assert(receiver.tracked.value == 41 && receiver.seed() == 7);
        assert(calls == 1 && frame.expired() && Tracked::alive == 1 && CallToken::alive == 0);
    }
    for (bool temporary : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        const Receiver receiver(41);
        auto done = std::make_shared<Done>();
        try {
            auto outcome = temporary ? retained_member(done) : receiver.accumulate(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive == (temporary ? 3 : 1));
                auto held = std::move(pending);
                if (mode == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("qualified member failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == (temporary ? 98 : 90));
        assert(receiver.tracked.value == 41 && receiver.seed() == 7);
        assert(calls == 1 && frame.expired() && Tracked::alive == 1 && CallToken::alive == 0);
    }
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        Receiver receiver(41);
        auto done = std::make_shared<Done>();
        try {
            auto outcome = receiver.accumulate(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(Tracked::alive == 2 && CallToken::alive == 0);
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("member failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 58);
        assert(receiver.tracked.value == (mode == 3 ? 41 : mode == 2 ? 42 : 44));
        assert(receiver.seed() == (mode == 3 ? 7 : mode == 2 ? 8 : 9));
        assert(calls == (mode == 3 ? 1 : 2));
        assert(frame.expired() && Tracked::alive == 1 && CallToken::alive == 0);
    }
    for (bool choose : {false, true}) for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        sequence = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_selection(choose, done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(Tracked::alive == (calls == 4 ? 3 : 2) && CallToken::alive == 0);
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 4)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("selection failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == (choose ? 98 : 99));
        assert(calls == (mode == 3 ? 1 : 4));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        Tracked borrowed(42);
        auto value = std::make_unique<Tracked>(41);
        auto forwarded = std::make_unique<int>(5);
        int* identity = forwarded.get();
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_parameters(std::move(value), borrowed, std::move(forwarded), done);
            assert(!value);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                assert(Tracked::alive == 2 && forwarded.get() == identity);
                auto held = std::move(pending);
                if (mode == 2) held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("parameter failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 131);
        assert(borrowed.value == (mode >= 2 ? 42 : 43));
        assert(forwarded.get() == identity && *forwarded == (mode >= 2 ? 5 : 47));
        assert(calls == 1 && frame.expired() && Tracked::alive == 1 && CallToken::alive == 0);
    }
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        sequence = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_receiver(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                while (pending) {
                    assert(sequence == (calls == 1 ? 1 : 12));
                    assert(Tracked::alive == (calls == 3 || calls == 5 ? 2 : 1));
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 5)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("receiver failure"))));
                    else held->resume_with(Result<void*>::success(new int(next_value)));
                }
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 183);
        assert(calls == (mode == 3 ? 1 : 5));
        assert(frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }
    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto target = &original_indirect_target;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_indirect_target(target, done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                target = &changed_indirect_target;
                auto held = std::move(pending);
                if (mode == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("indirect failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 142);
        assert(calls == 1 && frame.expired() && Tracked::alive == 0 && CallToken::alive == 0);
    }

    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        int borrowed = 7;
        expected_indirect_reference = &borrowed;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_indirect_reference(&reference_indirect_target, borrowed, done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                borrowed = 10;
                auto held = std::move(pending);
                if (mode == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("reference failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == (mode == 0 ? 50 : 53));
        assert(borrowed == (mode == 0 ? 8 : mode == 1 ? 11 : mode == 2 ? 10 : 7));
        assert(calls == 1 && frame.expired() && CallToken::alive == 0);
    }

    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_indirect_temporary(&temporary_indirect_target, done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                auto held = std::move(pending);
                if (mode == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("temporary failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 50);
        assert(calls == 1 && frame.expired() && CallToken::alive == 0);
    }

    for (mode = 0; mode < 4; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            auto outcome = retained_callback_body(done);
            if (intrinsics::is_coroutine_suspended(outcome)) {
                auto held = std::move(pending);
                if (mode == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("callback outer failure"))));
                else held->resume_with(Result<void*>::success(new int(next_value)));
            } else done->resume_with(Result<void*>::success(outcome));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 42);
        assert(calls == 1 && frame.expired() && CallToken::alive == 0);
    }

    for (bool implicit : {false, true}) for (bool dispatched : {false, true}) {
        int stages = 0;
        auto done = std::make_shared<Done>();
        auto dispatcher = std::make_shared<YieldDispatcher>();
        if (dispatched) done->context = dispatcher;
        auto outcome = implicit ? retained_implicit_yield(stages, done) : retained_plain_yield(stages, done);
        if (dispatched) {
            assert(intrinsics::is_coroutine_suspended(outcome));
            assert(stages == 1 && done->calls == 0 && Tracked::alive == 1);
            assert(dispatcher->yield_calls == 1 && dispatcher->queue.size() == 1);
            dispatcher->run_next();
            assert(stages == 2 && done->calls == 0 && Tracked::alive == 1);
            assert(dispatcher->yield_calls == 2 && dispatcher->queue.size() == 1);
            dispatcher->run_next();
            assert(dispatcher->queue.empty());
        } else {
            assert(!intrinsics::is_coroutine_suspended(outcome));
            done->resume_with(Result<void*>::success(outcome));
            assert(dispatcher->yield_calls == 0 && dispatcher->queue.empty());
        }
        assert(stages == 2 && done->calls == 1 && !done->failed && done->value == 42);
        assert(Tracked::alive == 0);
    }
    assert(to_delay_millis(kotlin::time::nanoseconds(std::numeric_limits<long long>::max())) == 9223372036854LL);
    assert(to_delay_millis(kotlin::time::nanoseconds(std::numeric_limits<long long>::min())) == 0);
    assert(to_delay_millis(kotlin::time::nanoseconds(1000001)) == 2);
    for (int kind = 0; kind < 4; ++kind) for (long long duration : {-1LL, 0LL, 1000001LL}) for (bool cancel : {false, true}) {
        if (cancel && duration <= 0) continue;
        int stages = 0;
        auto done = std::make_shared<Done>();
        auto dispatcher = std::make_shared<TimerDispatcher>();
        auto job = JobImpl::create(nullptr);
        done->context = job->operator+(dispatcher);
        auto outcome = retained_implicit_delay(kind, duration, stages, done);
        if (duration > 0) {
            assert(intrinsics::is_coroutine_suspended(outcome));
            assert(stages == 1 && done->calls == 0 && Tracked::alive == 1);
            assert(dispatcher->schedules == 1 && dispatcher->timer);
            assert(dispatcher->scheduled_millis == (kind >= 2 ? 2 : duration));
            dispatcher->fire();
            assert(done->calls == 0 && dispatcher->queue.size() == 1);
            if (cancel) job->cancel();
            dispatcher->run_next();
            if (!cancel) {
                assert(stages == 2 && done->calls == 0 && Tracked::alive == 1);
                assert(dispatcher->schedules == 2 && dispatcher->timer);
                dispatcher->fire();
                dispatcher->run_next();
            }
            assert(!dispatcher->timer && dispatcher->queue.empty());
        } else {
            assert(!intrinsics::is_coroutine_suspended(outcome));
            done->resume_with(Result<void*>::success(outcome));
            assert(dispatcher->schedules == 0);
        }
        assert(stages == (cancel ? 1 : 2) && done->calls == 1 && done->failed == cancel);
        if (cancel) {
            assert(done->failure);
            try { std::rethrow_exception(done->failure); }
            catch (const CancellationException&) {}
        }
        assert(cancel || done->value == 42);
        assert(Tracked::alive == 0);
    }
}
