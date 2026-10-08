#pragma once
#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>
#include <kotlinx/coroutines/Yield.hpp>
#include <kotlinx/coroutines/Delay.hpp>
#include <string>
#include <stdexcept>
#include <cassert>
using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::dsl;
struct Tracked {
    static int alive;
    static int throw_value;
    int value;
    explicit Tracked(int v) : value(v) {
        if (v == throw_value) throw std::runtime_error("array construction failure");
        ++alive;
    }
    Tracked(const Tracked&) = delete;
    ~Tracked() { --alive; }
};
struct CallToken {
    static int alive;
    int value;
    bool owns = true;
    explicit CallToken(int value) : value(value) { ++alive; }
    CallToken(const CallToken&) = delete;
    CallToken(CallToken&& other) : value(other.value), owns(std::exchange(other.owns, false)) {}
    ~CallToken() { if (owns) --alive; }
};
struct CatchValue {
    static int alive;
    static int copies;
    int value;
    explicit CatchValue(int value) : value(value) { ++alive; }
    CatchValue(const CatchValue& other) : value(other.value) { ++alive; ++copies; }
    ~CatchValue() { --alive; }
};
struct CatchContextObserver {
    std::exception_ptr expected;
    explicit CatchContextObserver(std::exception_ptr expected) : expected(std::move(expected)) {}
    CatchContextObserver(const CatchContextObserver&) = delete;
    ~CatchContextObserver() { assert(std::current_exception() == expected); }
};
struct CatchContextException {
    std::exception_ptr expected;
    explicit CatchContextException(std::exception_ptr expected) : expected(std::move(expected)) {}
    ~CatchContextException() { assert(std::current_exception() == expected); }
};
struct Range {
    static int alive;
    int values[3];
    explicit Range(int base) : values{base, base + 1, base + 2} { ++alive; }
    Range(const Range&) = delete;
    Range(Range&&) = delete;
    ~Range() { --alive; }
    int* begin() { return values; }
    int* end() { return values + 3; }
};
struct Condition {
    static int alive;
    int value;
    Condition(int value) : value(value) { ++alive; }
    Condition(const Condition&) = delete;
    ~Condition() { --alive; }
    explicit operator bool() const { return value > 0; }
};
struct Receiver {
    Tracked tracked;
    explicit Receiver(int value) : tracked(value) {}
    Receiver(const Receiver&) = delete;
    int add(int value) & { tracked.value += value; return tracked.value; }
    int take(int value) && { tracked.value += value; return tracked.value; }
    [[clang::annotate("suspend")]] void* await(int value, std::shared_ptr<Continuation<void*>> completion) &&;
    [[clang::annotate("suspend")]] void* accumulate(std::shared_ptr<Continuation<void*>> completion) &;
    [[clang::annotate("suspend")]] void* accumulate(std::shared_ptr<Continuation<void*>> completion) const &;
    [[clang::annotate("suspend")]] void* accumulate(std::shared_ptr<Continuation<void*>> completion) &&;
    [[clang::annotate("suspend")]] void* header_value(std::shared_ptr<Continuation<void*>> completion) const &;
    int seed() const { return seed_; }
private:
    int seed_ = 7;
    void advance() { ++seed_; }
    int read(void* raw) const & {
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        return tracked.value + seed_ + *boxed;
    }
};
extern int sequence;
extern bool ordered;
extern bool assignment_order;
extern bool dependent_order;
extern bool fail_second;
void sequence_event(int digit);
[[clang::annotate("suspend")]] void* external_call(int value, CallToken token, std::shared_ptr<Continuation<void*>> completion);
namespace authoring { inline namespace current { namespace nested {
void* retained_value(int value, std::shared_ptr<Continuation<void*>> completion);
void* retained_condition(std::shared_ptr<Continuation<void*>> completion);
void* retained_switch(std::shared_ptr<Continuation<void*>> completion);
void* retained_range(std::shared_ptr<Continuation<void*>> completion);
void* retained_condition_scope(std::shared_ptr<Continuation<void*>> completion);
void* retained_comma(std::shared_ptr<Continuation<void*>> completion);
void* retained_assignment(std::shared_ptr<Continuation<void*>> completion);
void* retained_selection(bool choose, std::shared_ptr<Continuation<void*>> completion);
void* retained_parameters(const std::unique_ptr<Tracked> value, Tracked& borrowed,
                          std::unique_ptr<int>&& forwarded, std::shared_ptr<Continuation<void*>> completion);
void* retained_receiver(std::shared_ptr<Continuation<void*>> completion);
void* retained_member(std::shared_ptr<Continuation<void*>> completion);
void* retained_exception(std::shared_ptr<Continuation<void*>> completion);
void* retained_rethrow(std::shared_ptr<Continuation<void*>> completion);
void* retained_value_catch(std::shared_ptr<Continuation<void*>> completion);
void* retained_handler_loop(std::shared_ptr<Continuation<void*>> completion);
void* retained_nested_context(std::shared_ptr<Continuation<void*>> completion);
void* retained_nested_exit(int choice, std::shared_ptr<Continuation<void*>> completion);
} } }
using authoring::nested::retained_value;
using authoring::nested::retained_condition;
using authoring::nested::retained_switch;
using authoring::nested::retained_range;
using authoring::nested::retained_condition_scope;
using authoring::nested::retained_comma;
using authoring::nested::retained_assignment;
using authoring::nested::retained_selection;
using authoring::nested::retained_parameters;
using authoring::nested::retained_receiver;
using authoring::nested::retained_member;
using authoring::nested::retained_exception;
using authoring::nested::retained_rethrow;
using authoring::nested::retained_value_catch;
using authoring::nested::retained_handler_loop;
using authoring::nested::retained_nested_context;
using authoring::nested::retained_nested_exit;
void* source_header_value(std::shared_ptr<Continuation<void*>> completion);
void* source_mixed_five(int value, std::shared_ptr<Continuation<void*>> completion);
void* source_mixed_five(double value, std::shared_ptr<Continuation<void*>> completion);
void* source_mixed_seven(int value, std::shared_ptr<Continuation<void*>> completion);
void* source_mixed_seven(double value, std::shared_ptr<Continuation<void*>> completion);

inline void* Receiver::header_value(std::shared_ptr<Continuation<void*>> completion) const & {
    Tracked local(5);
    void* raw = suspend(external_call(tracked.value, CallToken(tracked.value), completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + tracked.value + seed_ + local.value);
}
namespace authoring { inline namespace current { namespace nested {
[[suspend]] inline void* header_value(std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(5);
    void* raw = suspend(external_call(41, CallToken(41), completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value);
}
template <typename T>
[[suspend]] inline void* dependent_external(T& borrowed, std::shared_ptr<Continuation<void*>> completion) {
    return external_call(borrowed.value, CallToken(borrowed.value), completion);
}
struct DependentHost {
    template <typename T>
    [[suspend]] void* await(T& borrowed, std::shared_ptr<Continuation<void*>> completion) {
        return dependent_external(borrowed, completion);
    }
};
template <typename T>
[[suspend]] inline void* retained_template(T value, std::shared_ptr<Continuation<void*>> completion) {
    auto local = std::move(value);
    auto& alias = *local;
    const auto& borrowed = alias;
    auto&& forwarded = alias;
    const auto* pointer = &alias;
    decltype(auto) exact = (alias);
    decltype(auto) saved_alias = alias;
    decltype(auto) saved_pointer = pointer;
    DependentHost host;
    void* raw = host.await(alias, completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    assert(&alias == local.get() && &borrowed == local.get());
    assert(&forwarded == local.get() && pointer == local.get() && &exact == local.get());
    alias.value += 1;
    assert(borrowed.value == local->value && forwarded.value == local->value);
    assert(pointer->value == local->value && exact.value == local->value);
    pointer = nullptr;
    assert(saved_pointer == local.get() && &saved_alias == local.get());
    return new int(local->value + *boxed);
}
template <typename T>
class TemplateReceiver {
public:
    explicit TemplateReceiver(T value) : value_(std::move(value)) {}
    [[suspend]] void* run(std::shared_ptr<Continuation<void*>> completion) const & {
        Tracked local(5);
        void* raw = suspend(dependent_external(*value_, completion));
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        return new int(*boxed + value_->value + seed_ + local.value);
    }
    template <typename U>
    [[suspend]] void* run(U owned, std::shared_ptr<Continuation<void*>> completion) const &;
private:
    T value_;
    int seed_ = 3;
};
template <typename T>
template <typename U>
void* TemplateReceiver<T>::run(U owned, std::shared_ptr<Continuation<void*>> completion) const & {
    auto local = std::move(owned);
    void* raw = suspend(dependent_external(*value_, completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + value_->value + seed_ + local->value);
}
struct DependentReceiver {
    Tracked tracked{41};
    DependentReceiver& select() & { sequence_event(1); return *this; }
    [[suspend]] void* await(void* raw, std::shared_ptr<Continuation<void*>> completion) & {
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        sequence_event(2);
        return external_call(*boxed + tracked.value, CallToken(*boxed + tracked.value), completion);
    }
    [[suspend]] void* await(void* raw, std::shared_ptr<Continuation<void*>> completion) && {
        return static_cast<DependentReceiver&>(*this).await(raw, completion);
    }
};
template <typename T>
T make_dependent_receiver() { sequence_event(1); return T(); }
template <typename T>
[[suspend]] void* retained_owned_dependent_member(std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(5);
    void* raw = suspend(make_dependent_receiver<T>().await(suspend(external_call(41, CallToken(41), completion)), completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value);
}
template <typename T>
[[suspend]] void* retained_dependent_member(T& receiver, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(5);
    void* raw = suspend(receiver.select().await(suspend(external_call(41, CallToken(41), completion)), completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value);
}
[[suspend]] inline void* mixed_call(int value, std::shared_ptr<Continuation<void*>> completion) {
    return external_call(value, CallToken(value), completion);
}
inline void* mixed_call(double value, std::shared_ptr<Continuation<void*>>) {
    return new int(static_cast<int>(value * 2));
}
template <typename T, int Extra = 5>
[[suspend]] void* retained_mixed(T value, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(Extra);
    void* raw = mixed_call(value, completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value + (std::is_same_v<decltype(Extra), int> ? 0 : 1000));
}
template <typename T>
[[suspend]] void* retained_partial_mixed(T value, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(5);
    void* first_raw = suspend(external_call(0, CallToken(0), completion));
    std::unique_ptr<int> first(static_cast<int*>(first_raw));
    void* raw = mixed_call(value, completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value + *first);
}
struct UnknownSuspendReceiver {
    [[suspend]] void* await(std::shared_ptr<Continuation<void*>> completion) {
        return external_call(41, CallToken(41), completion);
    }
};
struct UnknownOrdinaryReceiver {
    void* await(std::shared_ptr<Continuation<void*>>) { return new int(72); }
};
class LateReceiver {
public:
    template <typename T>
    [[suspend]] void* run(T& receiver, std::shared_ptr<Continuation<void*>> completion) const & {
        Tracked local(bias_);
        void* first_raw = suspend(external_call(0, CallToken(0), completion));
        std::unique_ptr<int> first(static_cast<int*>(first_raw));
        void* raw = receiver.await(completion);
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        return new int(*boxed + local.value + *first);
    }
private:
    int bias_ = 5;
};
template <typename T>
class LateClassReceiver {
public:
    explicit LateClassReceiver(T& receiver) : receiver_(&receiver) {}
    [[suspend]] void* run(std::shared_ptr<Continuation<void*>> completion) const & {
        Tracked local(bias_);
        void* first_raw = suspend(external_call(0, CallToken(0), completion));
        std::unique_ptr<int> first(static_cast<int*>(first_raw));
        void* raw = receiver_->await(completion);
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        return new int(*boxed + local.value + *first);
    }
private:
    T* receiver_;
    int bias_ = 5;
};
template <typename T, int Bias = 5>
class NestedReceiver {
    using Payload = T;
public:
    class Entry {
    public:
        explicit Entry(Payload& receiver) : receiver_(&receiver) {}
        [[suspend]] void* run(std::shared_ptr<Continuation<void*>> completion) const & {
            Tracked local(Bias);
            void* first_raw = suspend(external_call(0, CallToken(0), completion));
            std::unique_ptr<int> first(static_cast<int*>(first_raw));
            void* raw = receiver_->await(completion);
            std::unique_ptr<int> boxed(static_cast<int*>(raw));
            return new int(*boxed + local.value + *first);
        }
        template <bool NoThrow>
        [[suspend]] void* checked(std::shared_ptr<Continuation<void*>> completion) const & noexcept(NoThrow) {
            Tracked local(Bias);
            void* first_raw = suspend(external_call(0, CallToken(0), completion));
            std::unique_ptr<int> first(static_cast<int*>(first_raw));
            void* raw = receiver_->await(completion);
            std::unique_ptr<int> boxed(static_cast<int*>(raw));
            return new int(*boxed + local.value + *first);
        }
    private:
        Payload* receiver_;
    };
};
template <typename T>
[[suspend]] void* retained_unknown_member(T& receiver, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(5);
    void* first_raw = suspend(external_call(0, CallToken(0), completion));
    std::unique_ptr<int> first(static_cast<int*>(first_raw));
    void* raw = receiver.await(completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value + *first);
}
template <typename T>
class ExplicitOuter {
public:
    template <typename U> class Entry;
};
template <>
template <>
class ExplicitOuter<int>::Entry<int> {
public:
    template <typename R>
    [[suspend]] void* run(R& receiver, std::shared_ptr<Continuation<void*>> completion) const & {
        Tracked local(bias_);
        void* first_raw = suspend(external_call(0, CallToken(0), completion));
        std::unique_ptr<int> first(static_cast<int*>(first_raw));
        void* raw = receiver.await(completion);
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        return new int(*boxed + local.value + *first);
    }
private:
    int bias_ = 5;
};
inline void* adl_call(...) { return new int(72); }
template <typename T>
[[suspend]] void* retained_adl(T value, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(5);
    void* first_raw = suspend(external_call(0, CallToken(0), completion));
    std::unique_ptr<int> first(static_cast<int*>(first_raw));
    void* raw = adl_call(value, completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value + *first);
}
namespace adl {
struct SuspendArgument {};
struct OrdinaryArgument {};
[[suspend]] inline void* adl_call(SuspendArgument, std::shared_ptr<Continuation<void*>> completion) {
    return external_call(41, CallToken(41), completion);
}
inline void* adl_call(OrdinaryArgument, std::shared_ptr<Continuation<void*>>) { return new int(72); }
}
namespace values {
inline int five = 5;
inline int seven = 7;
}
template <typename T, int* Bias>
[[suspend]] void* retained_pointer_argument(T value, int* expected, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(Bias ? *Bias : 5);
    void* raw = mixed_call(value, completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value + (Bias ? *Bias : 5) +
        (std::is_same_v<decltype(Bias), int*> && Bias == expected ? 0 : 10000));
}
template <typename T, int& Bias>
[[suspend]] void* retained_reference_argument(T value, int* expected, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(Bias);
    void* raw = mixed_call(value, completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value + Bias +
        (std::is_same_v<decltype(Bias), int&> && &Bias == expected ? 0 : 10000));
}
namespace targets {
[[suspend]] inline void* suspend_target(int value, std::shared_ptr<Continuation<void*>> completion) {
    return external_call(value, CallToken(value), completion);
}
inline void* ordinary_target(int value, std::shared_ptr<Continuation<void*>>) { return new int(value + 1); }
}
template <auto Target>
[[suspend]] void* retained_function_argument(int value, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(5);
    void* raw = Target(value, completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value +
        (std::is_same_v<decltype(Target), void* (*)(int, std::shared_ptr<Continuation<void*>>)> ? 0 : 10000));
}
template <auto& Target>
[[suspend]] void* retained_function_reference(int value, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(5);
    void* raw = Target(value, completion);
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + local.value +
        (std::is_same_v<decltype(Target), void* (&)(int, std::shared_ptr<Continuation<void*>>)> ? 0 : 10000));
}
template <auto Target>
class RetainedFunctionArgument {
public:
    [[suspend]] void* run(int value, std::shared_ptr<Continuation<void*>> completion) const & {
        Tracked local(5);
        void* raw = Target(value, completion);
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        return new int(*boxed + local.value +
            (std::is_same_v<decltype(Target), void* (*)(int, std::shared_ptr<Continuation<void*>>)> ? 0 : 10000));
    }
};
template <auto& Target>
class RetainedFunctionReference {
public:
    [[suspend]] void* run(int value, std::shared_ptr<Continuation<void*>> completion) const & {
        Tracked local(5);
        void* raw = Target(value, completion);
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        return new int(*boxed + local.value +
            (std::is_same_v<decltype(Target), void* (&)(int, std::shared_ptr<Continuation<void*>>)> ? 0 : 10000));
    }
};
} } }

// Compiler regression fixture: retain the indirect call target across argument suspension.
void* retained_indirect_target(int (*&target)(int), std::shared_ptr<Continuation<void*>> completion);

void* retained_indirect_reference(int (*target)(int&&, int), int& value, std::shared_ptr<Continuation<void*>> completion);

void* retained_indirect_temporary(int (*target)(const int&, int), std::shared_ptr<Continuation<void*>> completion);

void* retained_callback_body(std::shared_ptr<Continuation<void*>> completion);

void* retained_plain_yield(int& stages, std::shared_ptr<Continuation<void*>> completion);
void* retained_implicit_yield(int& stages, std::shared_ptr<Continuation<void*>> completion);
void* retained_implicit_delay(int kind, long long duration, int& stages, std::shared_ptr<Continuation<void*>> completion);

// A reference member makes assignment through a temporary receiver legal C++.
struct TemporaryFieldReceiver {
    Tracked tracked;
    int& value;
    explicit TemporaryFieldReceiver(int initial) : tracked(initial), value(tracked.value) {}
    TemporaryFieldReceiver(const TemporaryFieldReceiver&) = delete;
};
void* retained_field_assignment(std::shared_ptr<Continuation<void*>> completion);
