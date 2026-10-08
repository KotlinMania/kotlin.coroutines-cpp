// NOTE(port): C++ contract regression; this controllable mark is test infrastructure.
#include "kotlin/time/TimeSource.hpp"
#include <cassert>
#include <memory>
#include <stdexcept>
using namespace kotlin::time;

namespace {
class Mark final : public ComparableTimeMark {
public:
    Mark(int source, Duration reading) : source_(source), reading_(reading) {}
    Duration elapsed_now() const override { return -reading_; }
    Mark* plus(Duration duration) override { return new Mark(source_, reading_ + duration); }
    using ComparableTimeMark::minus;
    Duration minus(const ComparableTimeMark& other) const override {
        auto* mark = dynamic_cast<const Mark*>(&other);
        if (!mark || source_ != mark->source_) throw std::invalid_argument("different sources");
        return reading_ - mark->reading_;
    }
    // Equality is outside this test's scope; these test marks are not Any objects.
    bool equals(const kotlin::Any*) const override { return false; }
    std::int32_t hash_code() const override { return source_; }
private:
    int source_;
    Duration reading_;
};
class Source final : public TimeSource::WithComparableMarks {
public:
    Mark* mark_now() override { return new Mark(1, nanoseconds(12)); }
};
}

int main() {
    Source source;
    TimeSource& base_source = source;
    std::unique_ptr<TimeMark> original(base_source.mark_now());
    auto* comparable = dynamic_cast<ComparableTimeMark*>(original.get());
    assert(comparable);
    std::unique_ptr<ComparableTimeMark> earlier(comparable->minus(nanoseconds(3)));
    std::unique_ptr<ComparableTimeMark> same(comparable->minus(Duration::ZERO));
    assert(earlier->elapsed_now().compare_to(nanoseconds(-9)) == 0);
    kotlin::Comparable<ComparableTimeMark>& ordering = *comparable;
    assert(ordering.compare_to(*earlier) > 0);
    assert(earlier->compare_to(*comparable) < 0);
    assert(ordering.compare_to(*same) == 0);
    Mark unrelated(2, nanoseconds(12));
    bool propagated = false;
    try { (void)ordering.compare_to(unrelated); }
    catch (const std::invalid_argument&) { propagated = true; }
    assert(propagated);
}
