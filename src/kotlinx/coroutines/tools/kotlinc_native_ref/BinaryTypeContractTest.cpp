// Compiler result-family integration test. No IR classes or classifiers are fabricated.
#include "BinaryType.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

using namespace org::jetbrains::kotlin::backend::konan;
namespace collections = kotlin::collections;
namespace sequences = kotlin::sequences;

struct Borrowed {
    int value = 41;
};
struct Provider final : sequences::Sequence<Borrowed*> {
    struct Cursor final : collections::Iterator<Borrowed*> {
        explicit Cursor(std::shared_ptr<std::vector<Borrowed*>> values)
            : values_(std::move(values)) {}
        bool has_next() const override { return index_ != values_->size(); }
    protected:
        std::any next_dispatch() override {
            if (!has_next()) throw std::out_of_range("fixture exhausted");
            return (*values_)[index_++];
        }
    private:
        std::shared_ptr<std::vector<Borrowed*>> values_;
        std::size_t index_ = 0;
    };
    explicit Provider(bool once = false) : once_(once) {}
    std::shared_ptr<std::vector<Borrowed*>> values =
        std::make_shared<std::vector<Borrowed*>>();
    mutable unsigned requests = 0;
    static inline int destructions = 0;
    ~Provider() override { ++destructions; }
protected:
    std::unique_ptr<collections::detail::IteratorObject> iterator_dispatch() const override {
        if (once_ && requests != 0) throw std::logic_error("fixture constrained once");
        ++requests;
        return std::make_unique<Cursor>(values);
    }
private:
    const bool once_;
};

static_assert(std::is_abstract_v<sequences::Sequence<Borrowed*>>);
static_assert(std::is_convertible_v<sequences::Sequence<Borrowed*>*,
                                  sequences::Sequence<std::any>*>);
static_assert(std::is_final_v<BinaryType::Primitive>);
static_assert(std::is_final_v<BinaryType::Reference<Borrowed*>>);
static_assert(!std::is_default_constructible_v<BinaryType>);

int main() {
    constexpr std::array primitives{PrimitiveBinaryType::BOOLEAN, PrimitiveBinaryType::BYTE,
        PrimitiveBinaryType::SHORT, PrimitiveBinaryType::INT, PrimitiveBinaryType::LONG,
        PrimitiveBinaryType::FLOAT, PrimitiveBinaryType::DOUBLE, PrimitiveBinaryType::POINTER,
        PrimitiveBinaryType::VECTOR128};
    for (auto primitive : primitives) {
        BinaryType::Primitive result(primitive);
        assert(primitive_binary_type_or_null(result) == primitive);
    }
    Borrowed object;
    auto provider = std::make_shared<Provider>();
    provider->values->push_back(&object);
    std::weak_ptr<Provider> observer = provider;
    {
        BinaryType::Reference<Borrowed*> reference(provider, false);
        BinaryType::Reference<Borrowed*> nullable(reference.types(), true);
        assert(reference.types().get() == provider.get());
        assert(nullable.types().get() == reference.types().get());
        assert(!reference.nullable() && nullable.nullable());
        assert(!primitive_binary_type_or_null(reference));
        assert(!primitive_binary_type_or_null(nullable));
        assert(provider->requests == 0);
        // Mutation after result construction is observed when iteration starts.
        provider->values->push_back(&object);
        auto iterator = reference.types()->iterator();
        assert(provider->requests == 1 && iterator->next() == &object);
        object.value = 42;
        assert(iterator->next()->value == 42 && !iterator->has_next());
        std::shared_ptr<sequences::Sequence<std::any>> covariant = reference.types();
        auto wider = covariant->iterator();
        assert(provider->requests == 2);
        assert(std::any_cast<Borrowed*>(wider->next()) == &object);
        provider.reset();
        assert(!observer.expired() && Provider::destructions == 0);
    }
    assert(observer.expired() && Provider::destructions == 1);
    assert(object.value == 42);  // Sequence ownership never owns the borrowed element.
    auto once = std::make_shared<Provider>(true);
    BinaryType::Reference<Borrowed*> constrained(once, true);
    auto first = constrained.types()->iterator();
    assert(!first->has_next());
    bool rejected = false;
    try { constrained.types()->iterator(); }
    catch (const std::logic_error&) { rejected = true; }
    assert(rejected && once->requests == 1);
    std::cout << "binary result: nine primitives; lazy typed sequence; covariance; retained provider; borrowed elements\n";
}
