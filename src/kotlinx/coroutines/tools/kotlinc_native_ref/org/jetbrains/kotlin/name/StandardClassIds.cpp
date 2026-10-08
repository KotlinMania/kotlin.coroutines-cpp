// port-lint: source core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:8-371
#include "StandardClassIds.hpp"
namespace org::jetbrains::kotlin::name {
namespace {
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:344-344
ClassId base_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_kotlin_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:345-345
ClassId experimental_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_experimental_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:347-347
ClassId reflect_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_reflect_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:349-349
ClassId collections_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_collections_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:350-350
ClassId ranges_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_ranges_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:351-351
ClassId annotation_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_annotation_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:352-352
ClassId jvm_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_jvm_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:353-353
ClassId annotations_jvm_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_annotations_jvm_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:354-354
ClassId jvm_internal_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_jvm_internal_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:355-355
ClassId jvm_functions_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_jvm_functions_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:356-356
ClassId internal_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_internal_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:357-357
ClassId internal_ir_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_internal_ir_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:358-358
ClassId coroutines_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_coroutines_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:359-359
ClassId enums_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_enums_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:360-360
ClassId concurrent_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_concurrent_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:361-361
ClassId atomics_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_concurrent_atomics_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:362-362
ClassId sequences_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_sequences_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:364-364
ClassId test_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_test_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:369-369
ClassId js_id(const std::u16string& name) {
    return ClassId(StandardClassIds::base_js_package(), Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:346-346
ClassId unsigned_id(const ClassId& id) {
    return ClassId(StandardClassIds::base_kotlin_package(), Name::identifier(u"U" + id.short_class_name().get_identifier()));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:366-366
CallableId callable_id(const std::u16string& name, const FqName& package_name) {
    return CallableId(package_name, Name::identifier(name));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:367-367
CallableId callable_id(const std::u16string& name, const ClassId& class_id) {
    return CallableId(class_id, Name::identifier(name));
}
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:9-9
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_kotlin_package() {
    static const FqName VALUE = FqName(u"kotlin");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:10-10
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_reflect_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"reflect"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:11-11
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_experimental_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"experimental"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:12-12
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_collections_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"collections"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:13-13
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_sequences_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"sequences"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:14-14
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_ranges_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"ranges"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:15-15
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_jvm_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"jvm"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:16-16
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_js_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"js"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:17-17
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_annotations_jvm_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"annotations")).child(Name::identifier(u"jvm"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:18-18
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_jvm_internal_package() {
    static const FqName VALUE = StandardClassIds::base_jvm_package().child(Name::identifier(u"internal"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:19-19
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_jvm_functions_package() {
    static const FqName VALUE = StandardClassIds::base_jvm_package().child(Name::identifier(u"functions"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:20-20
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_annotation_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"annotation"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:21-21
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_internal_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"internal"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:22-22
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_internal_ir_package() {
    static const FqName VALUE = StandardClassIds::base_internal_package().child(Name::identifier(u"ir"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:23-23
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_coroutines_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"coroutines"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:24-24
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_coroutines_intrinsics_package() {
    static const FqName VALUE = StandardClassIds::base_coroutines_package().child(Name::identifier(u"intrinsics"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:25-25
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_enums_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"enums"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:26-26
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_contracts_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"contracts"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:27-27
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_concurrent_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"concurrent"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:28-28
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_concurrent_atomics_package() {
    static const FqName VALUE = StandardClassIds::base_concurrent_package().child(Name::identifier(u"atomics"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:29-29
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_test_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"test"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:30-30
// NOTE(port): Retained property initialized on first access.
const FqName& StandardClassIds::base_text_package() {
    static const FqName VALUE = StandardClassIds::base_kotlin_package().child(Name::identifier(u"text"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:50-50
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::nothing() {
    static const ClassId VALUE = base_id(u"Nothing");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:51-51
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::unit() {
    static const ClassId VALUE = base_id(u"Unit");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:52-52
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::any() {
    static const ClassId VALUE = base_id(u"Any");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:53-53
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::non_error() {
    static const ClassId VALUE = base_id(u"NonError");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:54-54
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::rich_error() {
    static const ClassId VALUE = base_id(u"RichError");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:55-55
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::enum_name() {
    static const ClassId VALUE = base_id(u"Enum");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:56-56
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::annotation() {
    static const ClassId VALUE = base_id(u"Annotation");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:57-57
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::array() {
    static const ClassId VALUE = base_id(u"Array");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:59-59
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::boolean() {
    static const ClassId VALUE = base_id(u"Boolean");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:60-60
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::char_name() {
    static const ClassId VALUE = base_id(u"Char");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:61-61
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::byte() {
    static const ClassId VALUE = base_id(u"Byte");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:62-62
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::short_name() {
    static const ClassId VALUE = base_id(u"Short");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:63-63
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::int_name() {
    static const ClassId VALUE = base_id(u"Int");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:64-64
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::long_name() {
    static const ClassId VALUE = base_id(u"Long");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:65-65
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::float_name() {
    static const ClassId VALUE = base_id(u"Float");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:66-66
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::double_name() {
    static const ClassId VALUE = base_id(u"Double");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:68-68
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::u_byte() {
    static const ClassId VALUE = unsigned_id(StandardClassIds::byte());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:69-69
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::u_short() {
    static const ClassId VALUE = unsigned_id(StandardClassIds::short_name());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:70-70
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::u_int() {
    static const ClassId VALUE = unsigned_id(StandardClassIds::int_name());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:71-71
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::u_long() {
    static const ClassId VALUE = unsigned_id(StandardClassIds::long_name());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:73-73
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::char_sequence() {
    static const ClassId VALUE = base_id(u"CharSequence");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:74-74
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::string() {
    static const ClassId VALUE = base_id(u"String");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:75-75
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::throwable() {
    static const ClassId VALUE = base_id(u"Throwable");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:77-77
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::cloneable() {
    static const ClassId VALUE = base_id(u"Cloneable");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:79-79
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_property() {
    static const ClassId VALUE = reflect_id(u"KProperty");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:80-80
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_mutable_property() {
    static const ClassId VALUE = reflect_id(u"KMutableProperty");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:81-81
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_property0() {
    static const ClassId VALUE = reflect_id(u"KProperty0");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:82-82
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_mutable_property0() {
    static const ClassId VALUE = reflect_id(u"KMutableProperty0");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:83-83
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_property1() {
    static const ClassId VALUE = reflect_id(u"KProperty1");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:84-84
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_mutable_property1() {
    static const ClassId VALUE = reflect_id(u"KMutableProperty1");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:85-85
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_property2() {
    static const ClassId VALUE = reflect_id(u"KProperty2");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:86-86
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_mutable_property2() {
    static const ClassId VALUE = reflect_id(u"KMutableProperty2");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:87-87
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_function() {
    static const ClassId VALUE = reflect_id(u"KFunction");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:88-88
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_class() {
    static const ClassId VALUE = reflect_id(u"KClass");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:89-89
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_callable() {
    static const ClassId VALUE = reflect_id(u"KCallable");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:90-90
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::k_type() {
    static const ClassId VALUE = reflect_id(u"KType");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:92-92
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::sequence() {
    static const ClassId VALUE = sequences_id(u"Sequence");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:94-94
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::comparable() {
    static const ClassId VALUE = base_id(u"Comparable");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:95-95
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::number() {
    static const ClassId VALUE = base_id(u"Number");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:97-97
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::function() {
    static const ClassId VALUE = base_id(u"Function");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:98-98
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::suspend_function() {
    static const ClassId VALUE = coroutines_id(u"SuspendFunction");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:116-116
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::continuation() {
    static const ClassId VALUE = coroutines_id(u"Continuation");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:117-117
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::coroutine_context() {
    static const ClassId VALUE = coroutines_id(u"CoroutineContext");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:139-139
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::iterator() {
    static const ClassId VALUE = collections_id(u"Iterator");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:140-140
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::iterable() {
    static const ClassId VALUE = collections_id(u"Iterable");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:141-141
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::collection() {
    static const ClassId VALUE = collections_id(u"Collection");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:142-142
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::list() {
    static const ClassId VALUE = collections_id(u"List");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:143-143
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::list_iterator() {
    static const ClassId VALUE = collections_id(u"ListIterator");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:144-144
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::set() {
    static const ClassId VALUE = collections_id(u"Set");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:145-145
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::map() {
    static const ClassId VALUE = collections_id(u"Map");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:146-146
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::abstract_map() {
    static const ClassId VALUE = collections_id(u"AbstractMap");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:147-147
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::mutable_iterator() {
    static const ClassId VALUE = collections_id(u"MutableIterator");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:148-148
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::char_iterator() {
    static const ClassId VALUE = collections_id(u"CharIterator");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:150-150
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::mutable_iterable() {
    static const ClassId VALUE = collections_id(u"MutableIterable");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:151-151
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::mutable_collection() {
    static const ClassId VALUE = collections_id(u"MutableCollection");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:152-152
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::mutable_list() {
    static const ClassId VALUE = collections_id(u"MutableList");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:153-153
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::mutable_list_iterator() {
    static const ClassId VALUE = collections_id(u"MutableListIterator");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:154-154
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::mutable_set() {
    static const ClassId VALUE = collections_id(u"MutableSet");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:155-155
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::mutable_map() {
    static const ClassId VALUE = collections_id(u"MutableMap");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:157-157
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::map_entry() {
    static const ClassId VALUE = StandardClassIds::map().create_nested_class_id(Name::identifier(u"Entry"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:158-158
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::mutable_map_entry() {
    static const ClassId VALUE = StandardClassIds::mutable_map().create_nested_class_id(Name::identifier(u"MutableEntry"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:160-160
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::result() {
    static const ClassId VALUE = base_id(u"Result");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:162-162
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::int_range() {
    static const ClassId VALUE = ranges_id(u"IntRange");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:163-163
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::long_range() {
    static const ClassId VALUE = ranges_id(u"LongRange");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:164-164
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::char_range() {
    static const ClassId VALUE = ranges_id(u"CharRange");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:166-166
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::annotation_retention() {
    static const ClassId VALUE = annotation_id(u"AnnotationRetention");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:167-167
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::annotation_target() {
    static const ClassId VALUE = annotation_id(u"AnnotationTarget");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:168-168
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::deprecation_level() {
    static const ClassId VALUE = base_id(u"DeprecationLevel");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:170-170
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::enum_entries() {
    static const ClassId VALUE = enums_id(u"EnumEntries");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:172-172
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::atomic_boolean() {
    static const ClassId VALUE = atomics_id(u"AtomicBoolean");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:173-173
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::atomic_int() {
    static const ClassId VALUE = atomics_id(u"AtomicInt");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:174-174
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::atomic_long() {
    static const ClassId VALUE = atomics_id(u"AtomicLong");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:175-175
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::atomic_reference() {
    static const ClassId VALUE = atomics_id(u"AtomicReference");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:183-183
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::atomic_array() {
    static const ClassId VALUE = atomics_id(u"AtomicArray");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:184-184
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::atomic_int_array() {
    static const ClassId VALUE = atomics_id(u"AtomicIntArray");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:185-185
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::atomic_long_array() {
    static const ClassId VALUE = atomics_id(u"AtomicLongArray");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:193-193
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::suppress() {
    static const ClassId VALUE = base_id(u"Suppress");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:194-194
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::published_api() {
    static const ClassId VALUE = base_id(u"PublishedApi");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:195-195
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::since_kotlin() {
    static const ClassId VALUE = base_id(u"SinceKotlin");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:196-196
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::extension_function_type() {
    static const ClassId VALUE = base_id(u"ExtensionFunctionType");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:197-197
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::context_function_type_params() {
    static const ClassId VALUE = base_id(u"ContextFunctionTypeParams");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:198-198
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::deprecated() {
    static const ClassId VALUE = base_id(u"Deprecated");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:199-199
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::deprecated_since_kotlin() {
    static const ClassId VALUE = base_id(u"DeprecatedSinceKotlin");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:200-200
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::require_kotlin() {
    static const ClassId VALUE = internal_id(u"RequireKotlin");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:201-201
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::dsl_marker() {
    static const ClassId VALUE = base_id(u"DslMarker");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:202-202
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::introduced_at() {
    static const ClassId VALUE = base_id(u"IntroducedAt");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:204-204
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::numeric_class() {
    static const ClassId VALUE = base_id(u"NumericClass");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:206-206
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::low_priority_in_overload_resolution() {
    static const ClassId VALUE = internal_id(u"LowPriorityInOverloadResolution");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:208-208
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::consistent_copy_visibility() {
    static const ClassId VALUE = base_id(u"ConsistentCopyVisibility");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:209-209
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::exposed_copy_visibility() {
    static const ClassId VALUE = base_id(u"ExposedCopyVisibility");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:211-211
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::hides_members() {
    static const ClassId VALUE = internal_id(u"HidesMembers");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:212-212
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::dynamic_extension() {
    static const ClassId VALUE = internal_id(u"DynamicExtension");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:213-213
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::intrinsic_const_evaluation() {
    static const ClassId VALUE = internal_id(u"IntrinsicConstEvaluation");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:215-215
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::retention() {
    static const ClassId VALUE = annotation_id(u"Retention");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:216-216
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::target() {
    static const ClassId VALUE = annotation_id(u"Target");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:217-217
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::repeatable() {
    static const ClassId VALUE = annotation_id(u"Repeatable");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:218-218
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::must_be_documented() {
    static const ClassId VALUE = annotation_id(u"MustBeDocumented");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:219-219
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::expect_refinement() {
    static const ClassId VALUE = experimental_id(u"ExpectRefinement");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:221-221
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::volatile_name() {
    static const ClassId VALUE = concurrent_id(u"Volatile");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:223-223
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::test() {
    static const ClassId VALUE = test_id(u"Test");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:225-225
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::raw_type_annotation() {
    static const ClassId VALUE = internal_ir_id(u"RawType");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:226-226
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::flexible_nullability() {
    static const ClassId VALUE = internal_ir_id(u"FlexibleNullability");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:227-227
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::flexible_mutability() {
    static const ClassId VALUE = internal_ir_id(u"FlexibleMutability");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:228-228
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::flexible_array_element_variance() {
    static const ClassId VALUE = internal_ir_id(u"FlexibleArrayElementVariance");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:229-229
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::enhanced_nullability() {
    static const ClassId VALUE = jvm_internal_id(u"EnhancedNullability");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:230-230
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::no_infer() {
    static const ClassId VALUE = internal_id(u"NoInfer");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:232-232
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::function_n() {
    static const ClassId VALUE = jvm_functions_id(u"FunctionN");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:234-234
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::inline_only() {
    static const ClassId VALUE = internal_id(u"InlineOnly");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:236-236
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::only_input_types() {
    static const ClassId VALUE = internal_id(u"OnlyInputTypes");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:238-238
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::restricts_suspension() {
    static const ClassId VALUE = coroutines_id(u"RestrictsSuspension");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:240-240
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::was_experimental() {
    static const ClassId VALUE = base_id(u"WasExperimental");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:242-242
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::must_use_return_values() {
    static const ClassId VALUE = base_id(u"MustUseReturnValues");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:243-243
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::ignorable_return_value() {
    static const ClassId VALUE = base_id(u"IgnorableReturnValue");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:245-245
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::equality_bound() {
    static const ClassId VALUE = base_id(u"EqualityBound");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:247-247
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::accessible_lateinit_property_literal() {
    static const ClassId VALUE = internal_id(u"AccessibleLateinitPropertyLiteral");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:249-249
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::optional_expectation() {
    static const ClassId VALUE = base_id(u"OptionalExpectation");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:250-250
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::implicitly_actualized_by_jvm_declaration() {
    static const ClassId VALUE = jvm_id(u"ImplicitlyActualizedByJvmDeclaration");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:251-251
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::kotlin_actual() {
    static const ClassId VALUE = annotations_jvm_id(u"KotlinActual");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:253-253
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::jvm_static() {
    static const ClassId VALUE = jvm_id(u"JvmStatic");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:254-254
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::jvm_name() {
    static const ClassId VALUE = jvm_id(u"JvmName");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:255-255
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::transient() {
    static const ClassId VALUE = jvm_id(u"Transient");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:257-257
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::js_export() {
    static const ClassId VALUE = js_id(u"JsExport");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:258-258
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::js_implicit_export() {
    static const ClassId VALUE = js_id(u"JsImplicitExport");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:259-259
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::js_export_ignore() {
    static const ClassId VALUE = StandardClassIds::Annotations::js_export().create_nested_class_id(Name::identifier(u"Ignore"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:260-260
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::js_export_default() {
    static const ClassId VALUE = StandardClassIds::Annotations::js_export().create_nested_class_id(Name::identifier(u"Default"));
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:261-261
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::js_no_dispatch_receiver() {
    static const ClassId VALUE = js_id(u"JsNoDispatchReceiver");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:262-262
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::js_no_runtime() {
    static const ClassId VALUE = js_id(u"JsNoRuntime");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:264-264
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::associated_object_key() {
    static const ClassId VALUE = reflect_id(u"AssociatedObjectKey");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:265-265
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::experimental_associated_objects() {
    static const ClassId VALUE = reflect_id(u"ExperimentalAssociatedObjects");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:269-269
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::jvm_builtin() {
    static const ClassId VALUE = internal_id(u"JvmBuiltin");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:270-270
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::suppress_bytecode_generation() {
    static const ClassId VALUE = internal_id(u"SuppressBytecodeGeneration");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:272-272
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::used_from_compiler_generated_code() {
    static const ClassId VALUE = internal_id(u"UsedFromCompilerGeneratedCode");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:280-280
// NOTE(port): Retained property initialized on first access.
const ClassId& StandardClassIds::Annotations::reflection_package_name() {
    static const ClassId VALUE = internal_id(u"ReflectionPackageName");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:283-283
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::value() {
    static const Name VALUE = Name::identifier(u"value");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:285-285
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::retention_value() {
    return StandardClassIds::Annotations::ParameterNames::value();
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:286-286
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::target_allowed_targets() {
    static const Name VALUE = Name::identifier(u"allowedTargets");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:288-288
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::equality_bound() {
    static const Name VALUE = Name::identifier(u"bound");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:290-290
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::since_kotlin_version() {
    static const Name VALUE = Name::identifier(u"version");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:292-292
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::actualizations() {
    static const Name VALUE = Name::identifier(u"actualizations");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:294-294
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::deprecated_message() {
    static const Name VALUE = Name::identifier(u"message");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:295-295
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::deprecated_level() {
    static const Name VALUE = Name::identifier(u"level");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:297-297
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::deprecated_since_kotlin_warning_since() {
    static const Name VALUE = Name::identifier(u"warningSince");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:298-298
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::deprecated_since_kotlin_error_since() {
    static const Name VALUE = Name::identifier(u"errorSince");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:299-299
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::deprecated_since_kotlin_hidden_since() {
    static const Name VALUE = Name::identifier(u"hiddenSince");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:301-301
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::suppress_names() {
    static const Name VALUE = Name::identifier(u"names");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:303-303
// NOTE(port): Retained property initialized on first access.
const Name& StandardClassIds::Annotations::ParameterNames::parameter_name_name() {
    static const Name VALUE = Name::identifier(u"name");
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:308-308
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::suspend() {
    static const CallableId VALUE = callable_id(u"suspend", StandardClassIds::base_kotlin_package());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:309-309
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::coroutine_context() {
    static const CallableId VALUE = callable_id(u"coroutineContext", StandardClassIds::base_coroutines_package());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:311-311
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::clone() {
    static const CallableId VALUE = callable_id(u"clone", StandardClassIds::cloneable());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:313-313
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::not_name() {
    static const CallableId VALUE = callable_id(u"not", StandardClassIds::boolean());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:315-315
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::contract() {
    static const CallableId VALUE = callable_id(u"contract", StandardClassIds::base_contracts_package());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:317-317
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::atomic_reference_compare_and_set() {
    static const CallableId VALUE = callable_id(u"compareAndSet", StandardClassIds::atomic_reference());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:318-318
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::atomic_reference_compare_and_exchange() {
    static const CallableId VALUE = callable_id(u"compareAndExchange", StandardClassIds::atomic_reference());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:319-319
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::atomic_array_compare_and_set_at() {
    static const CallableId VALUE = callable_id(u"compareAndSetAt", StandardClassIds::atomic_array());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:320-320
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::atomic_array_compare_and_exchange_at() {
    static const CallableId VALUE = callable_id(u"compareAndExchangeAt", StandardClassIds::atomic_array());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:322-322
// NOTE(port): Retained property initialized on first access.
const CallableId& StandardClassIds::Callables::throw_no_when_branch_matched_exception() {
    static const CallableId VALUE = callable_id(u"throwNoWhenBranchMatchedException", StandardClassIds::base_internal_package());
    return VALUE;
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:100-100
ClassId StandardClassIds::by_name(const std::u16string& name) { return base_id(name); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:101-101
ClassId StandardClassIds::reflect_by_name(const std::u16string& name) { return reflect_id(name); }
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:120-122
ClassId StandardClassIds::function_n(std::int32_t arity) {
    const auto digits = std::to_string(arity);
    return base_id(u"Function" + std::u16string(digits.begin(), digits.end()));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:125-127
ClassId StandardClassIds::suspend_function_n(std::int32_t arity) {
    const auto digits = std::to_string(arity);
    return coroutines_id(u"SuspendFunction" + std::u16string(digits.begin(), digits.end()));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:130-132
ClassId StandardClassIds::k_function_n(std::int32_t arity) {
    const auto digits = std::to_string(arity);
    return reflect_id(u"KFunction" + std::u16string(digits.begin(), digits.end()));
}
// Transliterated from: core/names/src/org/jetbrains/kotlin/name/StandardClassIds.kt:135-137
ClassId StandardClassIds::k_suspend_function_n(std::int32_t arity) {
    const auto digits = std::to_string(arity);
    return reflect_id(u"KSuspendFunction" + std::u16string(digits.begin(), digits.end()));
}
}
