// NOTE(port): C++ regression infrastructure for the translated TimeMark defaults.
// The controllable mark is a test double, not a replacement production clock.
#include "kotlin/time/TimeMark.hpp"
#include <cassert>
#include <memory>
#include <stdexcept>
using namespace kotlin::time;

namespace {
class ObservedMark final : public TimeMark {
public:
    explicit ObservedMark(int& destructions) : destructions_(destructions) {}
    ~ObservedMark() override { ++destructions_; }
    Duration elapsed_now() const override { return elapsed; }
    Duration elapsed = nanoseconds(0);
private:
    int& destructions_;
};
}

int main() {
    int destructions = 0;
    auto mark = std::make_shared<ObservedMark>(destructions);
    std::weak_ptr<ObservedMark> weak = mark;
    assert(mark->has_passed_now() && !mark->has_not_passed_now());
    auto ahead = std::unique_ptr<TimeMark>(mark->plus(nanoseconds(10)));
    assert(ahead->elapsed_now().compare_to(nanoseconds(-10)) == 0);
    assert(!ahead->has_passed_now() && ahead->has_not_passed_now());
    mark->elapsed = nanoseconds(15);
    assert(ahead->elapsed_now().compare_to(nanoseconds(5)) == 0);
    assert(ahead->has_passed_now() && !ahead->has_not_passed_now());
    auto adjusted_again = std::unique_ptr<TimeMark>(ahead->plus(nanoseconds(3)));
    auto adjusted_back = std::unique_ptr<TimeMark>(ahead->minus(nanoseconds(4)));
    assert(adjusted_again->elapsed_now().compare_to(nanoseconds(2)) == 0);
    assert(adjusted_back->elapsed_now().compare_to(nanoseconds(9)) == 0);
    mark.reset();
    ahead.reset();
    assert(!weak.expired() && destructions == 0);
    adjusted_again.reset();
    assert(!weak.expired());
    adjusted_back.reset();
    assert(weak.expired() && destructions == 1);

    // A borrowed source is not destroyed by its returned owning adjustment.
    {
        ObservedMark borrowed(destructions);
        borrowed.elapsed = nanoseconds(-1);
        assert(!borrowed.has_passed_now() && borrowed.has_not_passed_now());
        auto shifted = std::unique_ptr<TimeMark>(borrowed.minus(nanoseconds(2)));
        assert(shifted->elapsed_now().compare_to(nanoseconds(1)) == 0);
        shifted.reset();
        assert(destructions == 1);
        auto future = std::unique_ptr<TimeMark>(borrowed.plus(Duration::INFINITE));
        assert(future->elapsed_now().compare_to(-Duration::INFINITE) == 0);
        assert(future->has_not_passed_now());
        bool rejected = false;
        try {
            auto undefined = std::unique_ptr<TimeMark>(future->plus(-Duration::INFINITE));
        } catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }
    assert(destructions == 2);
}
