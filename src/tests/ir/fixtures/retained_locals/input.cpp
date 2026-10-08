#include "api.hpp"
#include <cassert>
[[suspend]] void* source_header_value(std::shared_ptr<Continuation<void*>> completion) {
    return authoring::nested::header_value(completion);
}
void* Receiver::await(int value, std::shared_ptr<Continuation<void*>> completion) && {
    return external_call(value, CallToken(value), completion);
}
void* Receiver::accumulate(std::shared_ptr<Continuation<void*>> completion) & {
    Receiver* identity = this;
    Tracked local(5);
    for (int index = 0; index < 2; ++index) {
        void* raw = suspend(external_call(index, CallToken(index), completion));
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        assert(this == identity);
        tracked.value += *boxed;
        advance();
    }
    return new int(this->tracked.value + seed_ + local.value);
}
void* Receiver::accumulate(std::shared_ptr<Continuation<void*>> completion) const & {
    return new int(read(suspend(external_call(tracked.value, CallToken(tracked.value), completion))));
}
void* Receiver::accumulate(std::shared_ptr<Continuation<void*>> completion) && {
    Tracked local(6);
    void* raw = suspend(external_call(tracked.value, CallToken(tracked.value), completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    advance();
    return new int(this->tracked.value + seed_ + *boxed + local.value);
}
namespace authoring { inline namespace current { namespace nested {
bool consume_condition(void* raw) {
    std::unique_ptr<int> value(static_cast<int*>(raw));
    return *value > 0;
}
int consume_value(void* raw) {
    std::unique_ptr<int> value(static_cast<int*>(raw));
    return *value;
}
[[suspend]] void* retained_condition(std::shared_ptr<Continuation<void*>> completion) {
    int iterations = 0;
    do {
        Tracked iteration(41);
        ++iterations;
        if (iterations == 1) continue;
    } while (consume_condition(suspend(external_call(2 - iterations, CallToken(2 - iterations), completion))));
    return new int(iterations);
}
[[suspend]] void* retained_value(int value, std::shared_ptr<Continuation<void*>> completion) {
    std::string prefix("retained");
    Tracked tracked(value);
    int values[2][2] = {{value, 1}, {2, 3}};
    const char text[4] = "cpp";
    Tracked members[2] = {Tracked(value), Tracked(value + 1)};
    int (&alias)[2][2] = values;
    alias[1][0] += 1;
    assert(sizeof(values) == 4 * sizeof(int));
    int accumulator = value + 1;
    int i = 0;
    int conditions = 0;
    do {
        Tracked iteration(value);
        void* raw = suspend(external_call(accumulator + i, CallToken(accumulator + i), completion));
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        accumulator += *boxed;
        values[0][1] += 1;
        assert(text[0] == 'c' && text[3] == '\0');
        ++i;
        if (i == 1) continue;
        if (i == 3) break;
    } while (++conditions < 4);
    assert(conditions == 2 && Tracked::alive == 3);
    return new int(accumulator + prefix.size() + tracked.value +
                   values[0][0] + values[0][1] + values[1][0] + values[1][1] +
                   members[0].value + members[1].value);
}
[[suspend]] void* retained_switch(std::shared_ptr<Continuation<void*>> completion) {
    int total = 0;
    for (int i = 0; i < 4; ++i) {
        Tracked outer(41);
        switch (Tracked selector(41); int choice = consume_value(suspend(external_call(i - 1, CallToken(i - 1), completion)))) {
        case 0: {
            Tracked inner(41);
            void* raw = suspend(external_call(i, CallToken(i), completion));
            std::unique_ptr<int> boxed(static_cast<int*>(raw));
            total += *boxed;
            continue;
        }
        case 1:
        case 2: {
            Tracked inner(41);
            void* raw = suspend(external_call(i, CallToken(i), completion));
            std::unique_ptr<int> boxed(static_cast<int*>(raw));
            total += *boxed;
            if (i == 1) break;
        }
        default: {
            Tracked inner(41);
            void* raw = suspend(external_call(i, CallToken(i), completion));
            std::unique_ptr<int> boxed(static_cast<int*>(raw));
            total += *boxed;
        }
        }
        assert(Tracked::alive == 1);
        total += 10;
    }
    assert(Tracked::alive == 0);
    return new int(total);
}
[[suspend]] void* retained_range(std::shared_ptr<Continuation<void*>> completion) {
    int values[3] = {1, 2, 3};
    int total = 0;
    for (int& item : values) {
        Tracked iteration(41);
        void* raw = suspend(external_call(item, CallToken(item), completion));
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        total += *boxed;
        item += 10;
        if (item == 11) continue;
        if (item == 13) break;
    }
    assert(values[0] == 11 && values[1] == 12 && values[2] == 13);
    for (int& item : Range(4)) {
        Tracked iteration(41);
        void* raw = suspend(external_call(item, CallToken(item), completion));
        std::unique_ptr<int> boxed(static_cast<int*>(raw));
        total += *boxed;
        ++item;
    }
    assert(Range::alive == 0 && Tracked::alive == 0);
    return new int(total);
}
void verify_increment(const Condition& choice, int& i, int resumed) {
    assert(Condition::alive == 1 && choice.value == 3 - i && Tracked::alive == 1);
    assert(resumed == i + 1);
    ++i;
}
[[suspend]] void* retained_condition_scope(std::shared_ptr<Continuation<void*>> completion) {
    int total = 0;
    if (Tracked guard(41); Condition choice = consume_value(suspend(external_call(0, CallToken(0), completion)))) {
        assert(Condition::alive == 1 && Tracked::alive == 1);
        total += choice.value;
    }
    assert(Condition::alive == 0 && Tracked::alive == 0);
    if (Tracked guard(41); Condition choice = consume_value(suspend(external_call(-1, CallToken(-1), completion)))) {
        total += 100;
    } else {
        assert(Condition::alive == 1 && choice.value == 0 && Tracked::alive == 1);
        total += 10;
    }
    assert(Condition::alive == 0 && Tracked::alive == 0);
    int i = 0;
    while (Condition choice = consume_value(suspend(external_call(2 - i, CallToken(2 - i), completion)))) {
        assert(Condition::alive == 1 && choice.value == 3 - i);
        total += choice.value;
        ++i;
        if (i == 1) continue;
        if (i == 2) break;
    }
    assert(Condition::alive == 0);
    i = 0;
    for (Tracked guard(41); Condition choice = consume_value(suspend(external_call(2 - i, CallToken(2 - i), completion))); verify_increment(choice, i, consume_value(suspend(external_call(i, CallToken(i), completion))))) {
        total += choice.value;
        if (i == 0) continue;
    }
    assert(i == 3 && Condition::alive == 0 && Tracked::alive == 0);
    i = 0;
    while (Condition choice = consume_value(suspend(external_call(1 - i, CallToken(1 - i), completion)))) {
        total += choice.value;
        ++i;
    }
    assert(i == 2 && Condition::alive == 0);
    return new int(total);
}
[[suspend]] void* retained_comma(std::shared_ptr<Continuation<void*>> completion) {
    int total = (Tracked(41), sequence_event(1), consume_value(suspend(external_call(0, CallToken(0), completion))));
    assert(Tracked::alive == 0);
    total += ((consume_value(suspend(external_call(1, CallToken(1), completion))), sequence_event(2)),
              consume_value(suspend(external_call(2, CallToken(2), completion))));
    int target = 41;
    int& alias = (Tracked(41), sequence_event(3), consume_value(suspend(external_call(3, CallToken(3), completion))), target);
    assert(Tracked::alive == 0);
    alias += total;
    assert(sequence == 123 && target == 45);
    if ((Tracked(41), consume_value(suspend(external_call(4, CallToken(4), completion))) > 0))
        assert(Tracked::alive == 0);
    return (Tracked(41), consume_value(suspend(external_call(5, CallToken(5), completion))), new int(target));
}
[[suspend]] void* retained_assignment(std::shared_ptr<Continuation<void*>> completion) {
    int values[3] = {10, 20, 30};
    int& alias = (values[consume_value(suspend(external_call(0, CallToken(0), completion)))] = (sequence_event(1), 7));
    alias += 1;
    assert(values[1] == 8 && &alias == &values[1]);
    values[consume_value(suspend(external_call(0, CallToken(0), completion)))] += (sequence_event(2), 5);
    assert(values[1] == 13);
    values[consume_value(suspend(external_call(0, CallToken(0), completion)))] =
        (sequence_event(3), consume_value(suspend(external_call(40, CallToken(40), completion))));
    assert(values[1] == 41 && sequence == 123);
    consume_value(suspend(external_call(0, CallToken(0), completion)))[(sequence_event(5), values)] = (sequence_event(4), 9);
    assert(values[1] == 9 && sequence == 12354);
    alias += 1;
    return new int(values[0] + values[1] + values[2]);
}
[[suspend]] void* retained_selection(bool choose, std::shared_ptr<Continuation<void*>> completion) {
    Tracked left(41);
    Tracked right(42);
    Tracked& selected = consume_value(suspend(external_call(choose ? 0 : -1, CallToken(choose ? 0 : -1), completion)))
        ? (consume_value(suspend(external_call(1, CallToken(1), completion))), left)
        : (consume_value(suspend(external_call(2, CallToken(2), completion))), right);
    assert(&selected == (choose ? &left : &right));
    selected.value += 10;
    std::unique_ptr<int> first(new int(5));
    std::unique_ptr<int> second(new int(6));
    std::unique_ptr<int> moved = choose
        ? (consume_value(suspend(external_call(3, CallToken(3), completion))), std::move(first))
        : (consume_value(suspend(external_call(4, CallToken(4), completion))), std::move(second));
    assert(moved && *moved == (choose ? 5 : 6));
    assert(!(choose ? first : second));
    choose
        ? (Tracked(41), static_cast<void>(consume_value(suspend(external_call(5, CallToken(5), completion)))), sequence_event(1))
        : (Tracked(41), static_cast<void>(consume_value(suspend(external_call(6, CallToken(6), completion)))), sequence_event(2));
    assert(sequence == (choose ? 1 : 2) && Tracked::alive == 2);
    return new int(left.value + right.value + *moved);
}
void verify_const(std::unique_ptr<Tracked>& value) = delete;
void verify_const(const std::unique_ptr<Tracked>& value) { assert(value && value->value == 41); }
[[suspend]] void* retained_parameters(const std::unique_ptr<Tracked> value, Tracked& borrowed,
                                    std::unique_ptr<int>&& forwarded, std::shared_ptr<Continuation<void*>> completion) {
    verify_const(value);
    void* raw = suspend(external_call(value->value, CallToken(value->value), completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    ++borrowed.value;
    *forwarded += *boxed;
    verify_const(value);
    return new int(value->value + borrowed.value + *forwarded);
}
Receiver& touch_receiver(Receiver& value) { sequence_event(1); return value; }
std::exception_ptr active_exception() { return std::current_exception(); }
[[noreturn]] void rethrow_active() { throw; }
[[suspend]] void* retained_exception(std::shared_ptr<Continuation<void*>> completion) {
    Tracked outer(41);
    int total = 0;
    try {
        Tracked protected_value(42);
        total = consume_value(suspend(external_call(0, CallToken(0), completion)));
    } catch (const std::logic_error& error) {
        return new int(900);
    } catch (const std::runtime_error& error) {
        assert(Tracked::alive == 1);
        Tracked handler(43);
        const std::runtime_error* identity = &error;
        std::exception_ptr cause = active_exception();
        assert(cause);
        int resumed = consume_value(suspend(external_call(1, CallToken(1), completion)));
        assert(&error == identity && active_exception() == cause);
        assert(std::string(error.what()).size() > 0 && Tracked::alive == 2);
        total = resumed + 5;
    }
    assert(Tracked::alive == 1 && !active_exception());
    return new int(total + outer.value);
}
[[suspend]] void* retained_rethrow(std::shared_ptr<Continuation<void*>> completion) {
    Tracked outer(41);
    try {
        try {
            Tracked protected_value(42);
            consume_value(suspend(external_call(0, CallToken(0), completion)));
        } catch (const std::runtime_error& error) {
            assert(Tracked::alive == 1);
            Tracked handler(43);
            std::exception_ptr cause = active_exception();
            assert(cause);
            consume_value(suspend(external_call(1, CallToken(1), completion)));
            assert(active_exception() == cause && std::string(error.what()).size() > 0);
            rethrow_active();
        }
    } catch (...) {
        assert(Tracked::alive == 1 && active_exception());
        return new int(77);
    }
    return new int(42);
}
[[suspend]] void* retained_value_catch(std::shared_ptr<Continuation<void*>> completion) {
    try {
        Tracked protected_value(41);
        consume_value(suspend(external_call(0, CallToken(0), completion)));
        throw CatchValue(7);
    } catch (CatchValue error) {
        assert(Tracked::alive == 0 && CatchValue::alive == 2 && CatchValue::copies == 1);
        std::exception_ptr cause = active_exception();
        assert(cause);
        consume_value(suspend(external_call(1, CallToken(1), completion)));
        assert(CatchValue::alive == 2 && CatchValue::copies == 1 && active_exception() == cause);
        ++error.value;
        return new int(error.value);
    }
}
[[suspend]] void* retained_member(std::shared_ptr<Continuation<void*>> completion) {
    void* raw = suspend(Receiver(41).accumulate(completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    assert(Tracked::alive == 1);
    return new int(*boxed + 1);
}
[[suspend]] void* retained_handler_loop(std::shared_ptr<Continuation<void*>> completion) {
    Tracked outer(41);
    for (int index = 0; index < 3; ++index) {
        assert(!active_exception());
        try {
            throw std::runtime_error("loop handler");
        } catch (...) {
            Tracked handler(42);
            std::exception_ptr cause = active_exception();
            assert(cause);
            consume_value(suspend(external_call(index, CallToken(index), completion)));
            assert(active_exception() == cause);
            if (index == 0) continue;
            if (index == 1) break;
        }
    }
    assert(!active_exception() && Tracked::alive == 1);
    return new int(42);
}
[[suspend]] void* retained_nested_context(std::shared_ptr<Continuation<void*>> completion) {
    Tracked outer(41);
    CatchContextObserver outside(std::exception_ptr{});
    try {
        throw std::runtime_error("outer context");
    } catch (const std::runtime_error& error) {
        Tracked handler(42);
        std::exception_ptr cause = active_exception();
        assert(cause);
        CatchContextObserver handler_context(cause);
        consume_value(suspend(external_call(0, CallToken(0), completion)));
        try {
            throw CatchContextException(cause);
        } catch (const CatchContextException& inner) {
            Tracked nested(43);
            std::exception_ptr inner_cause = active_exception();
            assert(inner_cause && inner_cause != cause);
            CatchContextObserver nested_context(inner_cause);
            consume_value(suspend(external_call(1, CallToken(1), completion)));
            assert(active_exception() == inner_cause && inner.expected == cause);
        }
        assert(active_exception() == cause && Tracked::alive == 2);
        consume_value(suspend(external_call(2, CallToken(2), completion)));
        assert(active_exception() == cause && std::string(error.what()) == "outer context");
    }
    assert(!active_exception() && Tracked::alive == 1);
    return new int(77);
}
Receiver* pointer_receiver(Receiver& value) { sequence_event(2); return &value; }
[[suspend]] void* retained_nested_exit(int choice, std::shared_ptr<Continuation<void*>> completion) {
    Tracked outer(41);
    CatchContextObserver outside(std::exception_ptr{});
    for (int index = 0; index < 2; ++index) {
        assert(!active_exception());
        try {
            throw std::runtime_error("outer exit");
        } catch (const std::runtime_error& error) {
            std::exception_ptr cause = active_exception();
            CatchContextObserver handler(cause);
            try {
                throw CatchContextException(cause);
            } catch (const CatchContextException& inner) {
                std::exception_ptr inner_cause = active_exception();
                CatchContextObserver nested(inner_cause);
                assert(inner.expected == cause);
                consume_value(suspend(external_call(index, CallToken(index), completion)));
                assert(active_exception() == inner_cause);
                if (choice == 0) return new int(77);
                if (choice == 1) break;
                continue;
            }
        }
    }
    assert(!active_exception());
    return new int(88);
}
[[suspend]] void* retained_receiver(std::shared_ptr<Continuation<void*>> completion) {
    Receiver receiver(41);
    int total = touch_receiver(receiver).add(consume_value(suspend(external_call(0, CallToken(0), completion))));
    assert(sequence == 1 && receiver.tracked.value == 42);
    total += pointer_receiver(receiver)->add(consume_value(suspend(external_call(1, CallToken(1), completion))));
    assert(sequence == 12 && receiver.tracked.value == 44);
    total += Receiver(41).take(consume_value(suspend(external_call(2, CallToken(2), completion))));
    assert(Tracked::alive == 1);
    total += std::move(receiver).take(consume_value(suspend(external_call(3, CallToken(3), completion))));
    assert(receiver.tracked.value == 48);
    total += consume_value(suspend(Receiver(41).await(4, completion)));
    assert(Tracked::alive == 1);
    return new int(total);
}
} } }

void* source_mixed_five(int value, std::shared_ptr<Continuation<void*>> completion) {
    return authoring::nested::retained_mixed<int, 5>(value, completion);
}
void* source_mixed_five(double value, std::shared_ptr<Continuation<void*>> completion) {
    return authoring::nested::retained_mixed<double, 5>(value, completion);
}
void* source_mixed_seven(int value, std::shared_ptr<Continuation<void*>> completion) {
    return authoring::nested::retained_mixed<int, 7>(value, completion);
}
void* source_mixed_seven(double value, std::shared_ptr<Continuation<void*>> completion) {
    return authoring::nested::retained_mixed<double, 7>(value, completion);
}

[[suspend]] void* retained_indirect_target(int (*&target)(int), std::shared_ptr<Continuation<void*>> completion) {
    return new int(target(authoring::nested::consume_value(
        suspend(external_call(41, CallToken(41), completion)))));
}

[[suspend]] void* retained_indirect_reference(int (*target)(int&&, int), int& value, std::shared_ptr<Continuation<void*>> completion) {
    return new int(target(std::move(value), authoring::nested::consume_value(
        suspend(external_call(41, CallToken(41), completion)))));
}

[[suspend]] void* retained_indirect_temporary(int (*target)(const int&, int), std::shared_ptr<Continuation<void*>> completion) {
    return new int(target(7, authoring::nested::consume_value(
        suspend(external_call(41, CallToken(41), completion)))));
}

[[suspend]] void* retained_callback_body(std::shared_ptr<Continuation<void*>> completion) {
    std::function<void*()> callback = [] { return static_cast<void*>(nullptr); };
    void* raw = suspend(external_call(41, CallToken(41), completion));
    assert(callback() == nullptr);
    return raw;
}

// Compiler regression: real yield calls need no suspend expression wrapper.
[[suspend]] void* retained_plain_yield(int& stages, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(40);
    for (int index = 0; index < 2; ++index) {
        ++stages;
        kotlinx::coroutines::yield(completion);
        ++local.value;
    }
    return new int(local.value);
}

// Compiler regression: Kotlin-style yield supplies the current frame implicitly.
[[suspend]] void* retained_implicit_yield(int& stages, std::shared_ptr<Continuation<void*>> completion) {
    Tracked local(40);
    for (int index = 0; index < 2; ++index) {
        ++stages;
        kotlinx::coroutines::yield();
        ++local.value;
    }
    return new int(local.value);
}

// Compiler regression: all duration forms retain locals through timer resumption.
[[suspend]] void* retained_implicit_delay(int completion, long long duration, int& stages, std::shared_ptr<Continuation<void*>> caller) {
    Tracked local(40);
    auto normalized_duration = kotlin::time::nanoseconds(duration);
    for (int index = 0; index < 2; ++index) {
        ++stages;
        if (completion == 0) kotlinx::coroutines::delay(duration);
        else if (completion == 1) kotlinx::coroutines::delay(kotlin::time::milliseconds(duration));
        else if (completion == 2) kotlinx::coroutines::delay(kotlin::time::nanoseconds(duration));
        else kotlinx::coroutines::delay(normalized_duration);
        ++local.value;
    }
    return new int(local.value);
}

// Kotlin receiver-before-value order, including an owned temporary receiver.
[[suspend]] void* retained_field_assignment(std::shared_ptr<Continuation<void*>> completion) {
    Receiver destination(5);
    authoring::nested::touch_receiver(destination).tracked.value = authoring::nested::consume_value(
        suspend(external_call(40, CallToken(40), completion)));
    assert(destination.tracked.value == 41 && sequence == 1);
    int assigned = ((sequence_event(2), TemporaryFieldReceiver(6)).value = authoring::nested::consume_value(
        suspend(external_call(41, CallToken(41), completion))));
    assert(assigned == 42 && sequence == 12 && Tracked::alive == 1);
    return new int(destination.tracked.value + assigned);
}
