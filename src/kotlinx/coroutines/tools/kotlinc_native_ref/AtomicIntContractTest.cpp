// Hand-authored contract tests for the translated Native AtomicInt dependency.
#include "kotlin/concurrent/atomics/Atomics.hpp"
#include <barrier>
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>
using namespace kotlin::concurrent::atomics;

int main() {
  std::cout << std::boolalpha;
  auto emit = [](const char* label, const auto& value) { std::cout << label << '=' << value << '\n'; };
  AtomicInt value(7);
  emit("initial", value.load());
  value.store(-4); emit("store", value.load());
  emit("exchange-old", value.exchange(9)); emit("exchange-new", value.load());
  emit("cas-false", value.compare_and_set(8, 100)); emit("cas-unchanged", value.load());
  emit("cas-true", value.compare_and_set(9, 12)); emit("cas-changed", value.load());
  emit("cx-fail-old", value.compare_and_exchange(9, 80)); emit("cx-fail-current", value.load());
  emit("cx-hit-old", value.compare_and_exchange(12, 15)); emit("cx-hit-current", value.load());
  emit("fetch-add-old", value.fetch_and_add(-20)); emit("fetch-add-new", value.load());
  emit("add-fetch", value.add_and_fetch(8));
  value.store(std::numeric_limits<std::int32_t>::max()); emit("wrap-plus", value.add_and_fetch(1));
  emit("wrap-minus", value.add_and_fetch(-1));
  value.store(0); minus_assign(value, std::numeric_limits<std::int32_t>::min()); emit("minus-min", value.load());
  plus_assign(value, -1); emit("plus-assign", value.load());
  emit("fetch-inc", fetch_and_increment(value)); emit("inc-fetch", increment_and_fetch(value));
  emit("dec-fetch", decrement_and_fetch(value)); emit("fetch-dec", fetch_and_decrement(value));
  value.store(20);
  int attempts = 0;
  emit("retry-old", fetch_and_update(value, [&](std::int32_t old) {
    ++attempts; if (attempts == 1) value.store(30); return old + 2;
  }));
  assert(attempts == 2 && value.load() == 32);
  emit("retry-attempts", attempts); emit("retry-new", value.load());
  emit("update-fetch", update_and_fetch(value, [](std::int32_t old) { return old * 2; }));
  update(value, [](std::int32_t old) { return old - 3; }); emit("update", value.load());
  try {
    update(value, [](std::int32_t) -> std::int32_t { throw std::logic_error("initializer"); });
    assert(false);
  } catch (const std::logic_error& error) { emit("transform-error", error.what()); }
  assert(value.load() == 61);
  emit("after-transform-error", value.load());
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  emit("legacy-set-old", value.get_and_set(4)); emit("legacy-add-old", value.get_and_add(2)); emit("legacy-add-new", value.add_and_get(3));
  emit("legacy-inc-old", value.get_and_increment()); emit("legacy-inc-new", value.increment_and_get());
  emit("legacy-dec-new", value.decrement_and_get()); emit("legacy-dec-old", value.get_and_decrement());
  value.set_value(-27); emit("property", value.get_value()); emit("text", value.to_string());
#pragma clang diagnostic pop
  value.store(0);
  update(value, [delta = std::make_unique<std::int32_t>(3)](std::int32_t old) { return old + *delta; });
  assert(value.load() == 3);
  emit("captured-update", value.load());

  value.store(0);
  std::vector<std::thread> workers;
  for (int i = 0; i < 4; ++i) workers.emplace_back([&] {
    for (int j = 0; j < 10000; ++j) value.fetch_and_add(1);
  });
  for (auto& worker : workers) worker.join();
  assert(value.load() == 40000); emit("concurrent-add", value.load());

  value.store(0); AtomicInt winners(0);
  std::barrier start(4);
  workers.clear();
  for (int i = 0; i < 4; ++i) workers.emplace_back([&] {
    start.arrive_and_wait(); if (value.compare_and_set(0, 1)) increment_and_fetch(winners);
  });
  for (auto& worker : workers) worker.join();
  assert(winners.load() == 1 && value.load() == 1); emit("cas-winners", winners.load());

  // Strong ordering forbids both loads seeing 0 in this store/load litmus.
  AtomicInt left(0), right(0);
  std::int32_t first = -1, second = -1;
  std::barrier phase(3);
  std::thread a([&] { for (int i = 0; i < 2000; ++i) {
    phase.arrive_and_wait(); left.store(1); first = right.load(); phase.arrive_and_wait();
  }});
  std::thread b([&] { for (int i = 0; i < 2000; ++i) {
    phase.arrive_and_wait(); right.store(1); second = left.load(); phase.arrive_and_wait();
  }});
  for (int i = 0; i < 2000; ++i) {
    left.store(0); right.store(0); phase.arrive_and_wait(); phase.arrive_and_wait();
    assert(first != 0 || second != 0);
  }
  a.join(); b.join();
}
