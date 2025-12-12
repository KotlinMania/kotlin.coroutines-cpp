#pragma once
#include <exception>
#include <variant>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {

template <typename T>
struct Result {
private:
    std::variant<T, std::exception_ptr> content;

public:
    Result() : content(std::exception_ptr(nullptr)) {} // Default failure or undefined? Prefer explicit.
    
    // Check if T is default constructible, if not we can't really default construct Result<T> safely without state.
    // Kotlin Result<T> is value class wrapping Any?.
    
    explicit Result(T v) : content(std::move(v)) {}
    explicit Result(std::exception_ptr e) : content(e) {}

    bool is_success() const { return std::holds_alternative<T>(content); }
    bool is_failure() const { return std::holds_alternative<std::exception_ptr>(content); }

    T get_or_throw() const {
        if (std::holds_alternative<std::exception_ptr>(content)) {
            std::rethrow_exception(std::get<std::exception_ptr>(content));
        }
        return std::get<T>(content);
    }
    
    std::exception_ptr exception_or_null() const {
        if (std::holds_alternative<std::exception_ptr>(content)) {
            return std::get<std::exception_ptr>(content);
        }
        return nullptr;
    }

    static Result<T> success(T value) { return Result<T>(std::move(value)); }
    static Result<T> failure(std::exception_ptr e) { return Result<T>(e); }
};

// Void specialization
template <>
struct Result<void> {
private:
    std::exception_ptr exception;

public:
    Result() : exception(nullptr) {}
    explicit Result(std::exception_ptr e) : exception(e) {}

    bool is_success() const { return !exception; }
    bool is_failure() const { return exception != nullptr; }

    void get_or_throw() const {
        if (exception) std::rethrow_exception(exception);
    }

    std::exception_ptr exception_or_null() const { return exception; }

    static Result<void> success() { return Result<void>(); }
    static Result<void> failure(std::exception_ptr e) { return Result<void>(e); }
};

} // namespace coroutines
} // namespace kotlinx
