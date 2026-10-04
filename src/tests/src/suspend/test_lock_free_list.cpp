/**
 * Regression coverage derived from:
 * kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt:61-249
 * kotlinx-coroutines-core/common/src/JobSupport.kt:451-568
 * These tests exercise publication, removal, closure, and completion registration races.
 */
#include "kotlinx/coroutines/internal/LockFreeLinkedList.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include <array>
#include <atomic>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::internal;

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}

void test_removal_and_closure_permissions() {
    LockFreeLinkedListHead head;
    LockFreeLinkedListNode first, second, rejected, completion, child;
    check(!first.remove(), "unlinked node must not report removal");
    check(head.add_one_if_empty(&first), "initial add failed");
    check(head.add_last(&second, LockFreeLinkedListNode::LIST_CHILD_PERMISSION), "second add failed");
    check(first.remove(), "first removal failed");
    check(!first.remove(), "duplicate removal reported success");
    check(first.is_removed(), "logical removal marker missing");
    check(head.next_node() == &second && second.prev_node() == &head, "removed predecessor not repaired");
    second.validate_node(&head, &head);
    check(second.remove() && head.is_empty(), "last removal did not restore empty list");
    head.close(LockFreeLinkedListNode::LIST_CANCELLATION_PERMISSION);
    check(!head.add_last(&rejected, LockFreeLinkedListNode::LIST_CANCELLATION_PERMISSION), "closed cancellation permission accepted");
    check(head.add_last(&completion, LockFreeLinkedListNode::LIST_ON_COMPLETION_PERMISSION), "closure blocked independent completion permission");
    head.close(LockFreeLinkedListNode::LIST_CHILD_PERMISSION);
    check(!head.add_last(&child, LockFreeLinkedListNode::LIST_CHILD_PERMISSION), "closed child permission accepted");
    head.close(LockFreeLinkedListNode::LIST_ON_COMPLETION_PERMISSION);
    check(!head.add_last(&rejected), "completion permission accepted after closure");
    check(!head.is_removed(), "sentinel was marked removed");
    bool threw = false;
    try { head.remove(); } catch (const std::logic_error&) { threw = true; }
    check(threw, "sentinel removal must fail");
}

void test_concurrent_publication_and_removal() {
    constexpr int NODE_COUNT = 4000;
    for (int round = 0; round < 20; ++round) {
        LockFreeLinkedListHead head;
        std::vector<std::unique_ptr<LockFreeLinkedListNode>> nodes;
        for (int i = 0; i < NODE_COUNT; ++i) nodes.push_back(std::make_unique<LockFreeLinkedListNode>());
        std::atomic<bool> start{false};
        std::atomic<bool> failed{false};
        std::atomic<int> removed{0};
        std::array<std::thread, 4> workers;
        for (int producer = 0; producer < 2; ++producer) {
            workers[producer] = std::thread([&, producer] {
                while (!start.load()) std::this_thread::yield();
                for (int i = producer; i < NODE_COUNT; i += 2) {
                    if (!head.add_last(nodes[i].get())) failed.store(true);
                }
            });
        }
        for (int consumer = 2; consumer < 4; ++consumer) {
            workers[consumer] = std::thread([&] {
                while (!start.load()) std::this_thread::yield();
                while (removed.load() < NODE_COUNT) {
                    auto* current = head.next_node();
                    while (current != &head) {
                        auto* next = current->next_node();
                        if (current->remove()) removed.fetch_add(1);
                        current = next;
                    }
                    std::this_thread::yield();
                }
            });
        }
        start.store(true);
        for (auto& worker : workers) worker.join();
        check(!failed.load(), "open list rejected an insertion");
        check(removed.load() == NODE_COUNT, "a published node was lost or removed twice");
        check(head.is_empty() && head.prev_node() == &head, "list links not repaired after concurrent removals");
        for (auto& node : nodes) check(node->is_removed(), "published node lacks a removal marker");
        // All nodes remain alive until every traversal and publication operation has finished.
    }
}

void test_handler_registration_races_completion() {
    constexpr int HANDLERS = 16;
    for (int round = 0; round < 1000; ++round) {
        auto job = make_job();
        std::array<std::atomic<int>, HANDLERS> calls{};
        std::atomic<bool> start{false};
        std::array<std::thread, 2> registrations;
        for (int producer = 0; producer < 2; ++producer) {
            registrations[producer] = std::thread([&, producer] {
                while (!start.load()) std::this_thread::yield();
                for (int i = producer; i < HANDLERS; i += 2) {
                    job->invoke_on_completion([&, i](std::exception_ptr) { calls[i].fetch_add(1); });
                }
            });
        }
        std::thread completion([&] {
            while (!start.load()) std::this_thread::yield();
            job->complete();
        });
        start.store(true);
        for (auto& registration : registrations) registration.join();
        completion.join();
        check(job->is_completed(), "job did not complete");
        for (auto& count : calls) check(count.load() == 1, "completion registration was lost or invoked twice");
    }
}
} // namespace

int main() {
    test_removal_and_closure_permissions();
    test_concurrent_publication_and_removal();
    test_handler_registration_races_completion();
    std::cout << "List publication, removal, closure and completion races passed\n";
}
