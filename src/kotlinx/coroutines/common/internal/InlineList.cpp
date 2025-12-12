#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/InlineList.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: @JvmInline value class needs C++ equivalent (std::variant or union)
// TODO: ArrayList needs std::vector equivalent
// TODO: Inline functions need proper C++ implementation
// TODO: assert function needs implementation

#include <vector>
#include <variant>
#include <functional>

namespace kotlinx {
    namespace coroutines {
        namespace {
            /*
 * Inline class that represents a mutable list, but does not allocate an underlying storage
 * for zero and one elements.
 * Cannot be parametrized with `List<*>`.
 */
            // TODO: @JvmInline - value class, using std::variant as approximation
            template<typename E>
            class InlineList {
            private:
                // TODO: holder can be: nullptr, E*, or std::vector<E*>*
                std::variant<std::monostate, E *, std::vector<E *> *> holder_;

            public:
                InlineList() : holder_(std::monostate{}) {
                }

                explicit InlineList(void *holder) {
                    // TODO: proper variant construction based on holder type
                    holder_ = std::monostate{};
                }

                InlineList<E> operator+(E *element) {
                    // TODO: assert { element !is List<*> } // Lists are prohibited

                    if (std::holds_alternative<std::monostate>(holder_)) {
                        // nullptr -> InlineList(element)
                        InlineList<E> result;
                        result.holder_ = element;
                        return result;
                    } else if (std::holds_alternative<std::vector<E *> *>(holder_)) {
                        // is ArrayList<*>
                        auto *list = std::get<std::vector<E *> *>(holder_);
                        list->push_back(element);
                        return *this;
                    } else {
                        // single element
                        auto *list = new std::vector<E *>();
                        list->reserve(4);
                        list->push_back(std::get<E *>(holder_));
                        list->push_back(element);
                        InlineList<E> result;
                        result.holder_ = list;
                        return result;
                    }
                }

                template<typename Action>
                void for_each_reversed(Action action) {
                    if (std::holds_alternative<std::monostate>(holder_)) {
                        return;
                    } else if (std::holds_alternative<E *>(holder_)) {
                        // !is ArrayList<*>
                        action(std::get<E *>(holder_));
                    } else {
                        // is ArrayList<*>
                        auto *list = std::get<std::vector<E *> *>(holder_);
                        for (int i = static_cast<int>(list->size()) - 1; i >= 0; --i) {
                            action((*list)[i]);
                        }
                    }
                }
            };
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx