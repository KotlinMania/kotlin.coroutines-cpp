/**
 * Regression cases derived from:
 * kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:501-605,1054-1187
 * A sender stored before receive forces the slow path that conditionally unwraps WaiterEB.
 */
#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::channels;

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}

void test_sender_first_rendezvous() {
    auto channel = std::make_shared<BufferedChannel<int>>(0);
    int completions = 0;
    auto sender = std::make_shared<FunctionalContinuation<void*>>(
        EmptyCoroutineContext::instance(), [&](Result<void*> result) {
            (void)result.get_or_throw();
            ++completions;
        });
    auto receiver = std::make_shared<FunctionalContinuation<void*>>(
        EmptyCoroutineContext::instance(), [](Result<void*>) {
            throw std::logic_error("immediate receive must return its value instead of resuming");
        });
    for (int value = 0; value < 128; ++value) {
        check(intrinsics::is_coroutine_suspended(channel->send(value, sender.get())), "sender did not suspend");
        auto result = channel->receive(receiver.get());
        check(!intrinsics::is_coroutine_suspended(result), "waiting sender did not rendezvous");
        std::unique_ptr<int> box(static_cast<int*>(result));
        check(box && *box == value, "rendezvous delivered the wrong value");
        check(completions == value + 1, "sender did not complete exactly once");
    }
    check(channel->close(nullptr), "channel did not close");
}

void test_receiver_first_rendezvous() {
    auto channel = std::make_shared<BufferedChannel<int>>(0);
    int completions = 0;
    int received = -1;
    auto receiver = std::make_shared<FunctionalContinuation<void*>>(
        EmptyCoroutineContext::instance(), [&](Result<void*> result) {
            std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
            check(box != nullptr, "resumed receive result is missing");
            received = *box;
            ++completions;
        });
    auto sender = std::make_shared<FunctionalContinuation<void*>>(
        EmptyCoroutineContext::instance(), [](Result<void*>) {
            throw std::logic_error("immediate send must return instead of resuming");
        });
    for (int value = 0; value < 128; ++value) {
        check(intrinsics::is_coroutine_suspended(channel->receive(receiver.get())), "receiver did not suspend");
        check(!intrinsics::is_coroutine_suspended(channel->send(value, sender.get())), "waiting receiver did not rendezvous");
        check(received == value && completions == value + 1, "receiver did not complete exactly once with the value");
    }
    check(channel->close(nullptr), "channel did not close");
}
} // namespace

int main() {
    test_sender_first_rendezvous();
    test_receiver_first_rendezvous();
    std::cout << "Sender-first and receiver-first rendezvous regressions passed\n";
}
