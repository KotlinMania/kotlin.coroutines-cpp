/**
 * Contract source: kotlinx-coroutines-core/native/src/internal/Concurrent.kt
 * Common extensions: kotlinx-coroutines-core/common/src/internal/Concurrent.common.kt
 * NOTE(port): C++ regression checks for Native identity, locking and borrowed references.
 */
#include "kotlinx/coroutines/internal/Concurrent.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

using namespace kotlinx::coroutines::internal;

static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

struct Value {
    int number;
};

int main() {
    try {
        // Concurrent.kt:11: Native does not interpret expectedSize as a reserve.
        auto negative = identity_set<Value*>(-1);
        auto large = identity_set<Value*>(2147483647);
        require(negative.empty() && large.empty(), "Native set size hints must be ignored");

        Value first{1}, equal{1}, second{2};
        negative.insert(&first);
        negative.insert(&equal);
        require(negative.size() == 2, "Distinct references must stay distinct");

        // Concurrent.kt:15-28 and AtomicReference reference comparison contract.
        WorkaroundAtomicReference<Value> reference(&first);
        require(!reference.compare_and_set(&equal, &second), "CAS must compare reference identity");
        require(reference.get() == &first, "Failed CAS must preserve the stored reference");
        require(reference.compare_and_set(&first, &second), "Matching CAS must succeed");
        require(reference.get_and_set(nullptr) == &second, "Exchange must return the old reference");
        require(get_value(reference) == nullptr, "Null references must be preserved");
        set_value(reference, &equal);
        require(get_value(reference) == &equal, "Value extension must forward to the actual field");

        // Concurrent.common.kt:36-40: each iteration reads the current field.
        int iterations = 0;
        struct Finished {};
        try {
            loop(reference, [&](auto& receiver, Value* value) {
                require(&receiver == &reference, "Loop must preserve receiver identity");
                if (iterations++ == 0) {
                    require(value == &equal, "First loop value differs");
                    receiver.set(&first);
                } else {
                    require(value == &first, "Loop must reload after the action changes the field");
                    throw Finished{};
                }
            });
        } catch (const Finished&) {}
        require(iterations == 2, "Loop action exception must propagate");

        // Concurrent.kt:9: the recursive lock is released during exception unwinding.
        ReentrantLock lock;
        int visits = 0;
        with_lock(lock, std::function<void()>([&] {
            visits += with_lock(lock, [] { return 1; });
        }));
        auto owned = with_lock(lock, [value = std::make_unique<Value>(Value{3})]() mutable {
            return std::move(value);
        });
        require(owned && owned->number == 3, "Lock action must preserve a move-only result");
        try {
            with_lock(lock, std::function<void()>([] { throw Finished{}; }));
        } catch (const Finished&) {}
        bool acquired = false;
        std::thread observer([&] {
            acquired = lock.try_lock();
            if (acquired) lock.unlock();
        });
        observer.join();
        require(visits == 1 && acquired, "with_lock must reenter and release after failure");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
