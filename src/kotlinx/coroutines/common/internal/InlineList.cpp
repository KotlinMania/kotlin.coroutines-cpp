// port-lint: source internal/InlineList.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/InlineList.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   @JvmInline internal value class InlineList<E>(private val holder: Any? = null) {
 *       operator fun plus(element: E): InlineList<E> { ... }
 *       inline fun forEachReversed(action: (E) -> Unit) { ... }
 *   }
 *
 * Kotlin's `@JvmInline value class` packs the single `holder` field at the call site;
 * the C++ port uses `std::variant<monostate, E*, vector<E*>*>` so the three states (empty
 * / single / list) are statically discriminated without a runtime tag overhead beyond the
 * variant's. The "lists are prohibited" invariant is enforced by typing — the variant
 * holds `E*`, never `List<E>*`, so misuse fails to compile rather than asserting at
 * runtime as Kotlin does.
 */

#include <cassert>
#include <functional>
#include <variant>
#include <vector>

namespace kotlinx::coroutines::internal {

template<typename E>
class InlineList {
public:
    InlineList() : holder_(std::monostate{}) {}

    explicit InlineList(E* single_element) : holder_(single_element) {}
    explicit InlineList(std::vector<E*>* list) : holder_(list) {}

    /**
     * Upstream:
     *   operator fun plus(element: E): InlineList<E> {
     *       assert { element !is List<*> } // Lists are prohibited
     *       ...
     *   }
     */
    InlineList<E> operator+(E* element) {
        if (std::holds_alternative<std::monostate>(holder_)) {
            return InlineList<E>(element);
        }
        if (std::holds_alternative<std::vector<E*>*>(holder_)) {
            auto* list = std::get<std::vector<E*>*>(holder_);
            list->push_back(element);
            return *this;
        }
        auto* list = new std::vector<E*>();
        list->reserve(4);
        list->push_back(std::get<E*>(holder_));
        list->push_back(element);
        return InlineList<E>(list);
    }

    /**
     * Upstream:
     *   inline fun forEachReversed(action: (E) -> Unit) {
     *       when (val holder = holder) {
     *           null -> return
     *           !is ArrayList<*> -> action(holder as E)
     *           else -> for (i in (holder as ArrayList<E>).indices.reversed()) action(holder[i])
     *       }
     *   }
     */
    template<typename Action>
    void for_each_reversed(Action action) {
        if (std::holds_alternative<std::monostate>(holder_)) {
            return;
        }
        if (std::holds_alternative<E*>(holder_)) {
            action(std::get<E*>(holder_));
            return;
        }
        auto* list = std::get<std::vector<E*>*>(holder_);
        for (int i = static_cast<int>(list->size()) - 1; i >= 0; --i) {
            action((*list)[i]);
        }
    }

private:
    std::variant<std::monostate, E*, std::vector<E*>*> holder_;
};

} // namespace kotlinx::coroutines::internal
