// Source contracts: native/src/Exceptions.kt:9-14 and
// libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:11-16.
#include "kotlinx/coroutines/Exceptions.hpp"
#include <memory>
#include <stdexcept>
#include <type_traits>

using ActualCancellation = kotlin::coroutines::cancellation::CancellationException;
static_assert(std::is_same_v<ActualCancellation, kotlinx::coroutines::CancellationException>);
static_assert(std::is_base_of_v<ActualCancellation, kotlinx::coroutines::JobCancellationException>);

namespace {
void require(bool value) {
    if (!value) throw std::runtime_error("Native cancellation namespace contract");
}
struct ResourceCause final : std::runtime_error {
    std::shared_ptr<int> resource;
    explicit ResourceCause(std::shared_ptr<int> value)
        : std::runtime_error("original cause"), resource(std::move(value)) {}
};
}

int main() {
    auto resource = std::make_shared<int>(101);
    std::weak_ptr<int> lifetime = resource;
    auto cause = std::make_exception_ptr(ResourceCause(resource));
    resource.reset();
    {
        // Factory construction occurs in the library translation unit; the result
        // has the actual stdlib class identity in this consumer translation unit.
        std::unique_ptr<ActualCancellation> exception(
            kotlinx::coroutines::cancellation_exception(std::nullopt, cause));
        cause = nullptr;
        require(!exception->get_message() && exception->get_cause());
        require(!lifetime.expired());
        auto original = exception->get_cause();
        try { throw *exception; }
        catch (const kotlinx::coroutines::CancellationException& caught) {
            require(caught.get_cause() == original && !caught.get_message());
            require(kotlinx::coroutines::is_cancellation_exception(std::current_exception()));
        }
        try { throw kotlinx::coroutines::CancellationException(""); }
        catch (const ActualCancellation& caught) {
            require(caught.get_message() && caught.get_message()->empty());
            require(!caught.get_cause());
        }
        auto copy = *exception;
        exception.reset();
        require(!lifetime.expired() && copy.get_cause() == original);
        require(copy.equals(&copy));
    }
    require(lifetime.expired());
    ActualCancellation first("same"), second("same");
    require(!first.equals(&second));
    require(first.hash_code() == first.hash_code());
    require(!ActualCancellation{}.get_message());
}
