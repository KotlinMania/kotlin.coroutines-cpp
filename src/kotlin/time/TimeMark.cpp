/**
 * Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt
 */
#include "kotlin/time/TimeMark.hpp"
#include <utility>

namespace kotlin::time {
namespace {
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:246-250
class AdjustedTimeMark final : public TimeMark {
public:
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:246
    AdjustedTimeMark(std::shared_ptr<TimeMark> mark, Duration adjustment)
        : mark_(std::move(mark)), adjustment_(adjustment) {}
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:247
    Duration elapsed_now() const override { return mark_->elapsed_now() - adjustment_; }
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:249
    TimeMark* plus(Duration duration) override {
        return new AdjustedTimeMark(mark_, adjustment_ + duration);
    }
private:
    const std::shared_ptr<TimeMark> mark_;
    const Duration adjustment_;
};
} // namespace

// NOTE(port): C++ virtual destruction releases the actual receiver resources.
TimeMark::~TimeMark() = default;

// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:164
TimeMark* TimeMark::plus(Duration duration) {
    auto owner = weak_from_this().lock();
    // NOTE(port): Keeping a borrowed receiver does not transfer ownership.
    if (!owner) owner = std::shared_ptr<TimeMark>(this, [](TimeMark*) {});
    return new AdjustedTimeMark(std::move(owner), duration);
}
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:177
TimeMark* TimeMark::minus(Duration duration) { return plus(-duration); }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:186
bool TimeMark::has_passed_now() const { return !elapsed_now().is_negative(); }
// Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:194
bool TimeMark::has_not_passed_now() const { return elapsed_now().is_negative(); }
} // namespace kotlin::time
