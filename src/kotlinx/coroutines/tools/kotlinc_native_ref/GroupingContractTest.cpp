// Source contracts: libraries/stdlib/src/kotlin/collections/Grouping.kt:27-32,77-87,139-145,186-192
#include "kotlin/collections/Grouping.hpp"
#include <array>
#include <cassert>
using namespace kotlin::collections;

// Instantiate actual abstract MutableMap interfaces; these functions are
// compile evidence only, not execution of a substitute map implementation.
template MutableMap<int, int>& kotlin::collections::aggregate_to<int, int, int>(
    const Grouping<int, int>&, MutableMap<int, int>&,
    const std::function<int(int, std::optional<int>, int, bool)>&);
template MutableMap<std::any, int>& kotlin::collections::fold_to<int, int, int>(
    const Grouping<int, int>&, MutableMap<std::any, int>&, int,
    const std::function<int(int, int)>&);
template MutableMap<int, int*>& kotlin::collections::fold_to<int, int, int*>(
    const Grouping<int, int>&, MutableMap<int, int*>&,
    const std::function<int*(int, int)>&,
    const std::function<int*(int, int*, int)>&);
template MutableMap<int, std::shared_ptr<int>>& kotlin::collections::aggregate_to<int, int, std::shared_ptr<int>>(
    const Grouping<int, int>&, MutableMap<int, std::shared_ptr<int>>&,
    const std::function<std::shared_ptr<int>(int, std::shared_ptr<int>, int, bool)>&);
template MutableMap<int, std::any>& kotlin::collections::aggregate_to<int, int, std::any>(
    const Grouping<int, int>&, MutableMap<int, std::any>&,
    const std::function<std::any(int, std::any, int, bool)>&);
template MutableMap<int, std::optional<int>>& kotlin::collections::aggregate_to<int, int, std::optional<int>>(
    const Grouping<int, int>&, MutableMap<int, std::optional<int>>&,
    const std::function<std::optional<int>(int, std::optional<int>, int, bool)>&);
template MutableMap<int, std::any>& kotlin::collections::reduce_to<std::any, int, int>(
    const Grouping<int, int>&, MutableMap<int, std::any>&,
    const std::function<std::any(int, std::any, int)>&);
template MutableMap<std::any, int>& kotlin::collections::each_count_to<int, int>(
    const Grouping<int, int>&, MutableMap<std::any, int>&);

namespace {
class Numbers final : public Iterator<int> {
public:
    bool has_next() const override { return next_ < 3; }
protected:
    std::any next_dispatch() override { return next_++; }
private:
    int next_ = 0;
};
class Parity final : public Grouping<int, int> {
protected:
    std::unique_ptr<detail::IteratorObject> source_iterator_dispatch() const override {
        return std::make_unique<Numbers>();
    }
    std::any key_of_dispatch(const std::any& element) const override {
        return std::any_cast<int>(element) % 2;
    }
};
}
int main() {
    Parity grouping;
    const Grouping<int, std::any>& covariant = grouping;
    assert(grouping.key_of(3) == 1);
    assert(std::any_cast<int>(covariant.key_of(3)) == 1);
    auto first = grouping.source_iterator();
    auto second = covariant.source_iterator();
    assert(first->next() == 0);
    assert(first->next() == 1);
    assert(second->next() == 0);
    assert(detail::grouping_accumulator<int>(std::optional<int>(0)) == 0);
    bool rejected = false;
    try { (void)detail::grouping_accumulator<int>(std::nullopt); }
    catch (const std::bad_any_cast&) { rejected = true; }
    assert(rejected);
    assert(detail::grouping_accumulator<int*>(nullptr) == nullptr);
    auto owner = std::make_shared<int>(8);
    assert(detail::grouping_accumulator<std::shared_ptr<int>>(owner).get() == owner.get());
    assert(!detail::grouping_accumulator<std::optional<int>>(std::nullopt).has_value());
    assert(!detail::grouping_accumulator<std::any>(std::any{}).has_value());
}
