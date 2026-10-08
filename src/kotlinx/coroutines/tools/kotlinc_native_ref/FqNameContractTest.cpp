// Source cases: compiler/tests/org/jetbrains/kotlin/name/FqNameUnsafeTest.java
// Additional cases exercise the class-ID/name API consumed by Native classification.
#include "org/jetbrains/kotlin/name/ClassId.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace org::jetbrains::kotlin::name;
namespace {
// Exercise the catalog-style initialization before main, across translation units.
const ClassId INITIAL_ID = ClassId::top_level(FqName(u"Initial"));
}
int main() {
    assert(INITIAL_ID.as_string() == u"Initial");
    assert(FqNameUnsafe(u"abc.def").starts_with(Name::identifier(u"abc")));
    assert(FqNameUnsafe(u"abc").starts_with(Name::identifier(u"abc")));
    assert(FqNameUnsafe(u"abc.").starts_with(Name::identifier(u"abc")));
    assert(FqNameUnsafe(u".abc").starts_with(Name::identifier(u"")));
    assert(!FqNameUnsafe(u"").starts_with(Name::identifier(u"")));
    assert(!FqNameUnsafe(u"").starts_with(Name::identifier(u"id")));
    assert(!FqNameUnsafe(u"segment").starts_with(Name::identifier(u"")));
    assert(!FqNameUnsafe(u".abc").starts_with(Name::identifier(u"abc")));
    assert(!FqNameUnsafe(u".abc").starts_with(Name::identifier(u"xyz")));
    assert(!FqNameUnsafe(u"abcdef").starts_with(Name::identifier(u"abc")));
    assert(!FqNameUnsafe(u"abc").starts_with(Name::identifier(u"abcdef")));
    assert(!FqNameUnsafe(u"abc.xyz").starts_with(Name::identifier(u"abcdef")));
    assert(!FqNameUnsafe(u"abc.def").starts_with(Name::special(u"<abc>")));
    assert(!FqNameUnsafe(u"abc").starts_with(Name::special(u"<abc>")));
    assert(!FqNameUnsafe(u"abc.").starts_with(Name::special(u"<abc>")));
    assert(!FqNameUnsafe(u".abc").starts_with(Name::special(u"<>")));
    assert(!FqNameUnsafe(u"").starts_with(Name::special(u"<>")));
    assert(!FqNameUnsafe(u"").starts_with(Name::special(u"<id>")));
    assert(!FqNameUnsafe(u"segment").starts_with(Name::special(u"<>")));
    assert(!FqNameUnsafe(u".abc").starts_with(Name::special(u"<abc>")));
    assert(!FqNameUnsafe(u"abcdef").starts_with(Name::special(u"<abc>")));
    assert(!FqNameUnsafe(u"abc").starts_with(Name::special(u"<abcdef>")));
    assert(!FqNameUnsafe(u"abc.xyz").starts_with(Name::special(u"<abcdef>")));
    assert(FqNameUnsafe(u"<abc>.def").starts_with(Name::special(u"<abc>")));
    assert(FqNameUnsafe(u"<abc>").starts_with(Name::special(u"<abc>")));
    assert(FqNameUnsafe(u"<abc>.").starts_with(Name::special(u"<abc>")));
    assert(FqNameUnsafe(u"<>.abc").starts_with(Name::special(u"<>")));
    assert(FqNameUnsafe(u"abc.def").starts_with(FqNameUnsafe(u"abc.def")));
    assert(FqNameUnsafe(u"abc.def").starts_with(FqNameUnsafe(u"abc")));
    assert(FqNameUnsafe(u"abc").starts_with(FqNameUnsafe(u"abc")));
    assert(FqNameUnsafe(u"abc.").starts_with(FqNameUnsafe(u"abc.")));
    assert(FqNameUnsafe(u"abc.").starts_with(FqNameUnsafe(u"abc")));
    assert(FqNameUnsafe(u"abc.def.").starts_with(FqNameUnsafe(u"abc")));
    assert(FqNameUnsafe(u"abc.def.").starts_with(FqNameUnsafe(u"abc.def")));
    assert(FqNameUnsafe(u"abc.def.").starts_with(FqNameUnsafe(u"abc.def.")));
    assert(FqNameUnsafe(u".abc").starts_with(FqNameUnsafe(u"")));
    assert(!FqNameUnsafe(u"").starts_with(FqNameUnsafe(u"")));
    assert(!FqNameUnsafe(u"").starts_with(FqNameUnsafe(u"id")));
    assert(!FqNameUnsafe(u"segment").starts_with(FqNameUnsafe(u"")));
    assert(!FqNameUnsafe(u".abc").starts_with(FqNameUnsafe(u"abc")));
    assert(!FqNameUnsafe(u".abc").starts_with(FqNameUnsafe(u"xyz")));
    assert(!FqNameUnsafe(u"abcdef").starts_with(FqNameUnsafe(u"abc")));
    assert(!FqNameUnsafe(u"abc").starts_with(FqNameUnsafe(u"abcdef")));
    assert(!FqNameUnsafe(u"abc").starts_with(FqNameUnsafe(u"abc.")));
    assert(!FqNameUnsafe(u"abc.def").starts_with(FqNameUnsafe(u"abc.")));
    assert(!FqNameUnsafe(u"abc.def").starts_with(FqNameUnsafe(u"abcdef")));
    assert(!FqNameUnsafe(u"abc.def").starts_with(FqNameUnsafe(u"abcxyz")));
    assert(!FqNameUnsafe(u"abc.def").starts_with(FqNameUnsafe(u"abc.xyz")));
    assert(!FqNameUnsafe(u"abc.def").starts_with(FqNameUnsafe(u"<abc>")));
    assert(!FqNameUnsafe(u"abc").starts_with(FqNameUnsafe(u"<abc>")));
    assert(!FqNameUnsafe(u"abc.").starts_with(FqNameUnsafe(u"<abc>")));
    assert(!FqNameUnsafe(u".abc").starts_with(FqNameUnsafe(u"<>")));
    assert(!FqNameUnsafe(u"").starts_with(FqNameUnsafe(u"<>")));
    assert(!FqNameUnsafe(u"").starts_with(FqNameUnsafe(u"<id>")));
    assert(!FqNameUnsafe(u"segment").starts_with(FqNameUnsafe(u"<>")));
    assert(!FqNameUnsafe(u".abc").starts_with(FqNameUnsafe(u"<abc>")));
    assert(!FqNameUnsafe(u"abcdef").starts_with(FqNameUnsafe(u"<abc>")));
    assert(!FqNameUnsafe(u"abc").starts_with(FqNameUnsafe(u"<abcdef>")));
    assert(!FqNameUnsafe(u"abc.xyz").starts_with(FqNameUnsafe(u"<abcdef>")));
    assert(!FqNameUnsafe(u"<abc>.def").starts_with(FqNameUnsafe(u"<abc>.")));
    assert(FqNameUnsafe(u"<abc>").starts_with(FqNameUnsafe(u"<abc>")));
    assert(FqNameUnsafe(u"<abc>.").starts_with(FqNameUnsafe(u"<abc>")));
    assert(FqNameUnsafe(u"<abc.>").starts_with(FqNameUnsafe(u"<abc.>")));
    assert(FqNameUnsafe(u"<>.abc").starts_with(FqNameUnsafe(u"<>")));
    assert(FqName::root().is_root() && FqName::root().to_string() == u"<root>");
    assert(FqName::root().short_name_or_special().is_special());
    for (int operation = 0; operation < 2; ++operation) {
        bool failed = false;
        try {
            if (operation) FqName::root().short_name();
            else FqName::root().parent();
        } catch (const std::logic_error& error) { failed = std::string(error.what()) == "root"; }
        assert(failed);
    }
    FqNameUnsafe quoted(u"foo.`inner.dot`");
    assert(quoted.parent().as_string() == u"foo");
    assert(quoted.short_name().as_string() == u"`inner.dot`");
    assert(quoted.parent().parent().is_root());
    auto built = FqName(u"foo").child(Name::identifier(u"inner.dot"));
    assert(built.short_name().as_string() == u"inner.dot");
    assert(built.parent().as_string() == u"foo");
    assert(FqName(built.as_string()).short_name().as_string() == u"dot");
    auto empty_child = FqName::root().child(Name::identifier(u""));
    assert(empty_child.is_root() && empty_child.parent().is_root());
    assert(empty_child.short_name().as_string().empty());
    FqNameUnsafe unsafe(u"<special>");
    assert(!unsafe.is_safe());
    auto direct = FqName(unsafe);
    assert(!unsafe.is_safe());
    auto safe = unsafe.to_safe();
    assert(unsafe.is_safe() && safe.as_string() == u"<special>");
    assert(&safe.as_string() == &unsafe.as_string());
    assert(&safe.to_unsafe().as_string() == &unsafe.as_string());
    assert(FqName(u"<special>").to_unsafe().is_safe());
    assert(!FqNameUnsafe::is_valid(std::nullopt));
    assert(!FqNameUnsafe::is_valid(u"bad/name") && !FqNameUnsafe::is_valid(u"bad*name"));
    assert(FqNameUnsafe::is_valid(u"`a.b`") && FqNameUnsafe::is_valid(u""));
    assert(FqName(u"abc").hash_code() == 96354);
    assert(FqName(u"包.子").equals(FqName(u"包.子")));
    assert(!FqName(u"abc").equals(FqNameUnsafe(u"abc")));
    assert(!FqNameUnsafe(u"abc").equals(FqName(u"abc")));
    FqName retained = FqName(u"survives").to_unsafe().to_safe();
    assert(retained.as_string() == u"survives");
    auto nested = ClassId::from_string(u"kotlin/Map.Entry", true);
    assert(nested.package_fq_name().as_string() == u"kotlin");
    assert(nested.relative_class_name().as_string() == u"Map.Entry");
    assert(nested.short_class_name().as_string() == u"Entry" && nested.is_nested_class());
    assert(nested.outer_class_id()->as_string() == u"kotlin/Map");
    assert(nested.outer_class_id()->is_local());
    assert(!nested.outermost_class_id().is_local());
    assert(nested.as_single_fq_name().as_string() == u"kotlin.Map.Entry");
    assert(nested.as_fq_name_string() == u"kotlin.Map.Entry");
    assert(nested.create_nested_class_id(Name::identifier(u"More")).as_string() == u"kotlin/Map.Entry.More");
    assert(nested.starts_with(Name::identifier(u"kotlin")));
    auto escaped = ClassId::from_string(u"foo/bar/`test/test`");
    assert(escaped.package_fq_name().as_string() == u"foo.bar");
    assert(escaped.relative_class_name().as_string() == u"test/test");
    assert(escaped.as_string() == u"foo/bar/`test/test`");
    assert(ClassId::from_string(escaped.as_string()).equals(escaped));
    auto root_class = ClassId::from_string(u"Root");
    assert(root_class.to_string() == u"/Root" && !root_class.outer_class_id());
    assert(root_class.as_fq_name_string() == u"Root");
    auto copied = nested.copy(std::nullopt, std::nullopt, false);
    assert(!copied.equals(nested) && copied.as_string() == nested.as_string());
    assert(copied.hash_code() - nested.hash_code() == 6);
    assert(copied.copy().equals(copied) && copied.copy().hash_code() == copied.hash_code());
    assert(copied.component1().as_string() == u"kotlin");
    assert(copied.component2().as_string() == u"Map.Entry" && !copied.component3());
    assert(ClassId::top_level(FqName(u"kotlin.Int")).as_string() == u"kotlin/Int");
    bool assertion = false;
    try { ClassId invalid(FqName(u"包"), FqName::root(), true); }
    catch (const std::logic_error& error) { assertion = std::string(error.what()) == "Class name must not be root: 包 (local)"; }
    assert(assertion);
    std::cout << "qualified names: upstream prefix cases; quoted dots; retained UTF-16 views; class-ID escaping/locality\n";
}
