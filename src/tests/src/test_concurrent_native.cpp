/**
 * Contract source: kotlinx-coroutines-core/native/src/internal/Concurrent.kt
 * Common extensions: kotlinx-coroutines-core/common/src/internal/Concurrent.common.kt
 * NOTE(port): C++ regression checks for Native identity, locking and borrowed references.
 */
#include "kotlinx/coroutines/internal/Concurrent.hpp"
#include "kotlinx/coroutines/concurrent/internal/OnDemandAllocatingPool.hpp"

#include <future>
#include <iostream>
#include <vector>
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

// OnDemandAllocatingPool.kt:48-92: reserved publication, capacity and closure.
static void pool_contract() {
    int created = 0;
    OnDemandAllocatingPool<int> pool(3, [&](int index) { ++created; return index + 10; });
    require(pool.state_representation() == "[]", "Initial pool state differs");
    for (int i = 0; i < 5; ++i) require(pool.allocate(), "Open/full pool must accept allocate");
    require(created == 3 && pool.state_representation() == "[10, 11, 12]", "Capacity/index order differs");
    require(pool.close() == std::vector<int>({10, 11, 12}), "close must return allocation-index order");
    require(pool.state_representation() == "[null, null, null][closed]", "Closed pool slots must be null");
    require(pool.to_string() == "OnDemandAllocatingPool([null, null, null][closed])", "Pool diagnostic differs");
    require(!pool.allocate() && pool.close().empty(), "Closed pool must reject allocation and repeated extraction");

    // Close waits for a reserved element's actual publication, then returns
    // the original owned resource. No borrowed pointer is adopted.
    std::promise<void> entered, publish;
    auto allow_publish = publish.get_future().share();
    auto resource = std::make_shared<Value>(Value{81});
    auto identity = resource.get();
    std::weak_ptr<Value> lifetime = resource;
    OnDemandAllocatingPool<std::shared_ptr<Value>> delayed(1, [&](int index) {
        require(index == 0, "Reserved slot index differs");
        entered.set_value(); allow_publish.wait(); return resource;
    });
    bool allocated = false;
    std::thread creator([&] { allocated = delayed.allocate(); });
    entered.get_future().wait();
    std::vector<std::shared_ptr<Value>> result;
    std::thread closer([&] { result = delayed.close(); });
    // Capacity is already reached. A false result establishes that close has
    // set the closed flag while publication is still blocked by the promise.
    while (delayed.allocate()) std::this_thread::yield();
    publish.set_value();
    creator.join(); closer.join();
    resource.reset();
    require(allocated && result.size() == 1 && result[0].get() == identity && result[0]->number == 81,
        "Reserved publication must preserve original resource identity");
    require(!lifetime.expired(), "Returned resource must remain owned");
    result.clear();
    require(lifetime.expired(), "Extracted slot must release its former owner");

    // Two concurrent closers must collectively extract every published slot once.
    OnDemandAllocatingPool<int> concurrent(32, [](int index) { return index; });
    std::vector<std::thread> allocators;
    for (int t = 0; t < 8; ++t) allocators.emplace_back([&] {
        for (int i = 0; i < 16; ++i) require(concurrent.allocate(), "Concurrent pool unexpectedly closed");
    });
    for (auto& thread : allocators) thread.join();
    std::vector<int> first, second;
    std::thread one([&] { first = concurrent.close(); });
    std::thread two([&] { second = concurrent.close(); });
    one.join(); two.join();
    require(first.empty() != second.empty(), "Exactly one closer must extract elements");
    auto& extracted = first.empty() ? second : first;
    require(extracted.size() == 32, "Concurrent extraction lost elements");
    for (int i = 0; i < 32; ++i) require(extracted[i] == i, "Concurrent allocation order differs");

    // The executable upstream body reserves before create. Its KDoc's
    // no-effect-on-exception claim conflicts with that body; preserve the body.
    auto creation_failure = std::make_exception_ptr(std::runtime_error("creation failure"));
    OnDemandAllocatingPool<int> failed(1, [creation_failure](int) -> int {
        std::rethrow_exception(creation_failure);
    });
    try {
        failed.allocate(); require(false, "Creation failure must escape");
    } catch (...) {
        require(std::current_exception() == creation_failure, "Creation exception identity differs");
    }
    require(failed.state_representation() == "[null]" && failed.allocate(),
        "Failed creation must preserve the upstream reserved slot, without an invented rollback");
    // close would spin on the unpublished slot, as in the source, so no close is invoked here.

    // Pool destruction releases owned slot boxes, even without explicit close.
    std::weak_ptr<Value> unclosed_lifetime;
    {
        OnDemandAllocatingPool<std::shared_ptr<Value>> unclosed(1, [&](int) {
            auto value = std::make_shared<Value>(Value{92}); unclosed_lifetime = value; return value;
        });
        require(unclosed.allocate() && !unclosed_lifetime.expired(), "Unclosed pool must retain its resource");
    }
    require(unclosed_lifetime.expired(), "Pool destruction must release owned slot storage");
}

int main() {
    try {
        pool_contract();
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
