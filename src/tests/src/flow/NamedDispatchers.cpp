// Original: kotlinx-coroutines-core/common/test/flow/NamedDispatchers.kt
// TODO: Translate imports to proper C++ includes
// TODO: Implement CoroutineDispatcher base class
// TODO: Implement CoroutineContext
// TODO: Implement Runnable interface/concept
// TODO: Implement operator() overloading for invoke

#include <string>
#include <vector>
#include <stdexcept>
#include <optional>
// TODO: #include proper headers

namespace kotlinx {
    namespace coroutines {
        /**
 * Test dispatchers that emulate multiplatform context tracking.
 */
        class NamedDispatchers {
        private:
            static ArrayStack &stack_instance() {
                static ArrayStack stack;
                return stack;
            }

            class NamedDispatcher : public CoroutineDispatcher {
            private:
                std::string name_;

            public:
                NamedDispatcher(const std::string &name) : name_(name) {
                }

                void dispatch(CoroutineContext &context, Runnable &block) override {
                    stack_instance().push(name_);
                    try {
                        block.run();
                    } catch (...) {
                        std::optional<std::string> last = stack_instance().pop();
                        if (!last.has_value()) {
                            throw std::runtime_error("No names on stack");
                        }
                        if (last.value() != name_) {
                            throw std::runtime_error(
                                "Inconsistent stack: expected " + name_ + ", but had " + last.value()
                            );
                        }
                        throw;
                    }
                    std::optional<std::string> last = stack_instance().pop();
                    if (!last.has_value()) {
                        throw std::runtime_error("No names on stack");
                    }
                    if (last.value() != name_) {
                        throw std::runtime_error(
                            "Inconsistent stack: expected " + name_ + ", but had " + last.value()
                        );
                    }
                }
            };

        public:
            static std::string name() {
                std::optional<std::string> result = stack_instance().peek();
                if (!result.has_value()) {
                    throw std::runtime_error("No names on stack");
                }
                return result.value();
            }

            static std::string name_or(const std::string &default_value) {
                std::optional<std::string> result = stack_instance().peek();
                return result.value_or(default_value);
            }

            static CoroutineDispatcher *operator()(const std::string &name) {
                return named(name);
            }

        private:
            static CoroutineDispatcher *named(const std::string &name) {
                return new NamedDispatcher(name);
            }
        };

        class ArrayStack {
        private:
            std::vector<std::string> elements_;
            size_t head_;

        public:
            ArrayStack() : elements_(16), head_(0) {
            }

            void push(const std::string &value) {
                if (elements_.size() == head_ - 1) {
                    ensure_capacity();
                }
                elements_[head_++] = value;
            }

            std::optional<std::string> peek() {
                if (head_ == 0) {
                    return std::nullopt;
                }
                return elements_[head_ - 1];
            }

            std::optional<std::string> pop() {
                if (head_ == 0) {
                    return std::nullopt;
                }
                return elements_[--head_];
            }

        private:
            void ensure_capacity() {
                size_t current_size = elements_.size();
                size_t new_capacity = current_size << 1;
                std::vector<std::string> new_elements(new_capacity);

                // elements.copyInto(destination = newElements, startIndex = head)
                for (size_t i = head_; i < current_size; ++i) {
                    new_elements[i - head_] = elements_[i];
                }

                // elements.copyInto(destination = newElements, destinationOffset = elements.size - head, endIndex = head)
                for (size_t i = 0; i < head_; ++i) {
                    new_elements[current_size - head_ + i] = elements_[i];
                }

                elements_ = std::move(new_elements);
            }
        };
    } // namespace coroutines
} // namespace kotlinx