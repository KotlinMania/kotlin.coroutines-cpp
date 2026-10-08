// Source cases: compiler/tests/org/jetbrains/kotlin/name/FqNameUnsafeTest.java
// Additional cases exercise the class-ID/name API consumed by Native classification.
#include "org/jetbrains/kotlin/name/ClassId.hpp"
#include "org/jetbrains/kotlin/name/CallableId.hpp"
#include "org/jetbrains/kotlin/name/SpecialNames.hpp"
#include "org/jetbrains/kotlin/name/StandardClassIds.hpp"
#include "org/jetbrains/kotlin/name/NativeRuntimeNames.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace org::jetbrains::kotlin::name;
namespace {
// Exercise the catalog-style initialization before main, across translation units.
const ClassId INITIAL_ID = ClassId::top_level(FqName(u"Initial"));
const CallableId INITIAL_LOCAL(Name::identifier(u"local"));
}
int main() {
    assert(INITIAL_ID.as_string() == u"Initial");
    assert(StandardClassIds::u_int().as_string() == u"kotlin/UInt");
    assert(StandardClassIds::map_entry().as_string() == u"kotlin/collections/Map.Entry");
    assert(StandardClassIds::mutable_map_entry().as_string() == u"kotlin/collections/MutableMap.MutableEntry");
    assert(StandardClassIds::function_n(23).as_string() == u"kotlin/Function23");
    assert(StandardClassIds::suspend_function_n(2).as_string() == u"kotlin/coroutines/SuspendFunction2");
    assert(StandardClassIds::k_function_n(-1).as_string() == u"kotlin/reflect/KFunction-1");
    assert(StandardClassIds::k_suspend_function_n(0).as_string() == u"kotlin/reflect/KSuspendFunction0");
    assert(StandardClassIds::by_name(u"Int").equals(StandardClassIds::int_name()));
    assert(StandardClassIds::reflect_by_name(u"KClass").equals(StandardClassIds::k_class()));
    assert(StandardClassIds::continuation().as_string() == u"kotlin/coroutines/Continuation");
    assert(StandardClassIds::Callables::coroutine_context().to_string() == u"kotlin/coroutines/coroutineContext");
    assert(StandardClassIds::Annotations::restricts_suspension().as_string() == u"kotlin/coroutines/RestrictsSuspension");
    assert(&StandardClassIds::Annotations::ParameterNames::value() == &StandardClassIds::Annotations::ParameterNames::retention_value());
    assert(StandardClassIds::Annotations::js_export_ignore().as_string() == u"kotlin/js/JsExport.Ignore");
    assert(StandardClassIds::Annotations::volatile_name().as_string() == u"kotlin/concurrent/Volatile");
    assert(StandardClassIds::atomic_int().as_string() == u"kotlin/concurrent/atomics/AtomicInt");
    assert(NativeRuntimeNames::atomic_int().as_string() == u"kotlin/concurrent/AtomicInt");
    assert(!StandardClassIds::atomic_reference().equals(NativeRuntimeNames::atomic_reference()));
    assert(NativeRuntimeNames::Callables::atomic_array().to_string() == u"kotlin/concurrent/AtomicArray");
    assert(!NativeRuntimeNames::Callables::atomic_array().class_id());
    assert(NativeRuntimeNames::Callables::atomic_array_compare_and_set().to_string() == u"kotlin/concurrent/AtomicArray.compareAndSet");
    assert(StandardClassIds::Callables::atomic_array_compare_and_set_at().to_string() == u"kotlin/concurrent/atomics/AtomicArray.compareAndSetAt");
    assert(NativeRuntimeNames::Callables::atomic_reference_compare_and_exchange().class_id()->equals(NativeRuntimeNames::atomic_reference()));
    assert(NativeRuntimeNames::Annotations::escapes_nothing().as_string() == u"kotlin/native/internal/escapeAnalysis/Escapes.Nothing");
    assert(NativeRuntimeNames::Annotations::throws().as_string() == u"kotlin/Throws");
    assert(NativeRuntimeNames::Annotations::throws_alias().as_string() == u"kotlin/native/Throws");
    assert(NativeRuntimeNames::Annotations::thread_local_name().as_string() == u"kotlin/native/concurrent/ThreadLocal");
    assert(NativeRuntimeNames::Annotations::thread_local_alias().as_string() == u"kotlin/native/ThreadLocal");
    assert(NativeRuntimeNames::Annotations::gc_unsafe_call_class_id().as_string() == u"kotlin/native/internal/GCUnsafeCall");
    assert(INITIAL_LOCAL.is_local() && INITIAL_LOCAL.to_string() == u"<local>/local");
    const auto callable_name = Name::identifier(u"bar");
    const CallableId top(FqName(u"one.two"), callable_name);
    const CallableId member(FqName(u"one.two"), FqName(u"A.B"), callable_name);
    assert(!top.is_local() && !top.class_name() && !top.class_id());
    assert(top.to_string() == u"one/two/bar" && top.as_single_fq_name().as_string() == u"one.two.bar");
    assert(member.to_string() == u"one/two/A.B.bar");
    assert(member.class_id()->as_string() == u"one/two/A.B");
    assert(member.as_single_fq_name().as_string() == u"one.two.A.B.bar");
    const CallableId local_class(ClassId(FqName(u"one.two"), FqName(u"A.B"), true), callable_name);
    assert(local_class.is_local() && local_class.equals(member));
    assert(local_class.hash_code() == member.hash_code());
    const CallableId debug(Name::identifier(u"loc"), FqName(u"one.two.foo"));
    assert(debug.is_local() && debug.as_fq_name_for_debug_info().as_string() == u"one.two.foo.loc");
    assert(debug.as_single_fq_name().as_string() == u"<local>.loc");
    const CallableId no_debug(Name::identifier(u"loc"));
    assert(debug.equals(no_debug) && debug.hash_code() == no_debug.hash_code());
    assert(!debug.equals(top) && !debug.equals(std::any{}) && !debug.equals(17));
    const auto renamed = debug.copy(Name::identifier(u"renamed"));
    assert(renamed.as_fq_name_for_debug_info().as_string() == u"one.two.foo.renamed");
    assert(renamed.is_local() && !renamed.equals(debug));
    const auto moved_class = with_class_id(debug, ClassId::top_level(FqName(u"pkg.C")));
    assert(!moved_class.is_local() && moved_class.as_fq_name_for_debug_info().as_string() == u"pkg.C.loc");
    assert(package_name(nullptr).as_string() == u"<local>" && is_local(nullptr));
    assert(package_name(&top).as_string() == u"one.two" && !is_local(&top));
    assert(CallableId(FqName::root(), callable_name).to_string() == u"/bar");
    assert(&SpecialNames::local() == &SpecialNames::local());
    assert(SpecialNames::anonymous_fq_name().as_string() == u"<anonymous>");
    assert(SpecialNames::this_name().as_string() == u"<this>");
    assert(SpecialNames::subscribe_operator_index(0).as_string() == u"<index_0>");
    bool bad_index = false;
    try { SpecialNames::subscribe_operator_index(-1); }
    catch (const std::invalid_argument& error) { bad_index = std::string(error.what()) == "Index should be non-negative, but was -1"; }
    assert(bad_index);
    const auto anonymous_parameter = SpecialNames::anonymous_parameter_name(-1);
    assert(anonymous_parameter.as_string() == u"<anonymous parameter -1>");
    assert(SpecialNames::is_anonymous_parameter_name(anonymous_parameter));
    assert(SpecialNames::is_anonymous_parameter_name(Name::special(u"<anonymous parameterSuffix>")));
    assert(!SpecialNames::is_anonymous_parameter_name(Name::identifier(u"anonymous parameter 1")));
    assert(SpecialNames::safe_identifier(static_cast<const Name*>(nullptr)).equals(SpecialNames::safe_identifier_for_no_name()));
    assert(SpecialNames::safe_identifier(&callable_name).equals(callable_name));
    const std::u16string empty_identifier;
    assert(SpecialNames::safe_identifier(&empty_identifier).as_string().empty());
    assert(!SpecialNames::is_safe_identifier(Name::identifier(u"")));
    assert(SpecialNames::is_safe_identifier(callable_name));
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
