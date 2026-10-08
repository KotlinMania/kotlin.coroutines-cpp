// Source contracts: core/compiler.common/src/org/jetbrains/kotlin/builtins/PrimitiveType.kt:11-58
// Publication contract: libraries/stdlib/jvm/src/kotlin/util/LazyJVM.kt:114-133
#include "org/jetbrains/kotlin/builtins/PrimitiveType.hpp"
#include "org/jetbrains/kotlin/builtins/StandardNames.hpp"
#include <array>
#include <barrier>
#include <cassert>
#include <thread>
#include <vector>
using namespace org::jetbrains::kotlin::builtins;
using org::jetbrains::kotlin::name::FqName;

int main() {
    // Race the first read, before any other use initializes these properties.
    constexpr int READERS = 16;
    std::barrier start(READERS);
    std::array<const FqName*, READERS> types{};
    std::array<const FqName*, READERS> arrays{};
    std::vector<std::thread> readers;
    for (int i = 0; i < READERS; ++i) {
        readers.emplace_back([&, i] {
            start.arrive_and_wait();
            types[i] = &PrimitiveType::CHAR.type_fq_name();
            arrays[i] = &PrimitiveType::CHAR.array_type_fq_name();
        });
    }
    for (auto& reader : readers) reader.join();
    for (int i = 0; i < READERS; ++i) {
        assert(types[i] == types[0]);
        assert(arrays[i] == arrays[0]);
    }
    assert(types[0]->as_string() == u"kotlin.Char");
    assert(arrays[0]->as_string() == u"kotlin.CharArray");
    assert(types[0] != arrays[0]);
    assert(StandardNames::built_ins_package_name().as_string() == u"kotlin");
    assert(StandardNames::built_ins_package_fq_name().as_string() == u"kotlin");
    const std::array<const PrimitiveType*, 8> primitives = {
        &PrimitiveType::BOOLEAN, &PrimitiveType::CHAR, &PrimitiveType::BYTE,
        &PrimitiveType::SHORT, &PrimitiveType::INT, &PrimitiveType::FLOAT,
        &PrimitiveType::LONG, &PrimitiveType::DOUBLE};
    const std::array<std::u16string, 8> names = {
        u"Boolean", u"Char", u"Byte", u"Short", u"Int", u"Float", u"Long", u"Double"};
    for (std::size_t i = 0; i < names.size(); ++i) {
        assert(primitives[i]->type_name().as_string() == names[i]);
        assert(primitives[i]->array_type_name().as_string() == names[i] + u"Array");
        assert(PrimitiveType::get_by_short_name(names[i]) == primitives[i]);
        assert(PrimitiveType::get_by_short_array_name(names[i] + u"Array") == primitives[i]);
        assert(primitives[i]->type_fq_name().as_string() == u"kotlin." + names[i]);
        assert(primitives[i]->array_type_fq_name().as_string() == u"kotlin." + names[i] + u"Array");
        assert(PrimitiveType::get_by_short_name(names[i] + u"Array") == nullptr);
        assert(PrimitiveType::get_by_short_array_name(names[i]) == nullptr);
    }
    for (const auto& invalid : {u"", u"int", u"kotlin.Int", u"UInt", u"Vector128", u"IntArrayArray"}) {
        assert(PrimitiveType::get_by_short_name(invalid) == nullptr);
        assert(PrimitiveType::get_by_short_array_name(invalid) == nullptr);
    }
}
