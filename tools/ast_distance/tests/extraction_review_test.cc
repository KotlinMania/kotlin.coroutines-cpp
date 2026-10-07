#include "ast_parser.hpp"
#include "cpp_review.hpp"
#include "symbol_analysis.hpp"
#include "imports.hpp"
#include "callable_identity.hpp"
#include "codebase.hpp"
#include "logic_similarity.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <unistd.h>
using namespace ast_distance;
int main(int argc, char** argv) {
    ASTParser parser;
    const std::string cpp = R"(
struct Base { virtual int abstract() = 0; };
struct Example : Base {
  Example() {}
  ~Example() {}
  int inline_method() { return 1; }
  int declaration();
  bool operator==(const Example&) const { return true; }
  operator bool() const { return true; }
};
int Example::declaration() { return 2; }
void ns::Qualified::command(int x) { (void)x; }
void ns::Qualified::command(double x) { (void)x; }
)";
    auto functions = parser.extract_function_infos(cpp, Language::CPP);
    assert(functions.size() == 8);
    std::multiset<std::string> names;
    for (const auto& f : functions) { assert(f.name != "<anonymous>"); names.insert(f.name); }
    assert(names.count("Example") == 1 && names.count("~Example") == 1);
    assert(names.count("operator==") == 1 && names.count("operator bool") == 1);
    assert(names.count("command") == 2 && names.count("inline_method") == 1);
    assert(functions.back().qualified_name == "ns::Qualified::command");
    assert(functions[6].signature != functions[7].signature);
    auto reference_returns = parser.extract_function_infos(R"(
namespace compiler {
const Visibility& Visibility::normalize() const { return *this; }
int& Record::get(int index) { return values[index]; }
Payload&& Factory::move(Payload& value) { return static_cast<Payload&&>(value); }
const Name& get_name(const Owner& owner) { return owner.name; }
}
)", Language::CPP);
    assert(!parser.last_extraction_has_errors() && reference_returns.size() == 4);
    assert(reference_returns[0].name == "normalize");
    assert(reference_returns[0].qualified_name == "Visibility::normalize");
    assert(reference_returns[0].signature == "()" && reference_returns[0].explicit_parameter_count == 0);
    assert(!reference_returns[0].is_namespace_function);
    assert(reference_returns[1].name == "get" && reference_returns[1].explicit_parameter_count == 1);
    assert(reference_returns[1].qualified_name == "Record::get");
    assert(reference_returns[2].name == "move" && reference_returns[2].first_parameter_type == "Payload");
    assert(reference_returns[2].qualified_name == "Factory::move");
    assert(reference_returns[3].name == "get_name" && reference_returns[3].is_namespace_function);
    assert(reference_returns[3].first_parameter_type == "Owner");
    std::string kotlin = R"(fun interface SharingStarted {
 companion object {
  fun WhileSubscribed(stop: Long = 0): SharingStarted = Started(stop)
 }
 fun command(count: Int): Int
}
fun SharingStarted.Companion.WhileSubscribed(stop: Duration = Duration.ZERO): SharingStarted = Started(stop.inWholeMilliseconds)
class Started {
 fun command(count: Int): Int = count
 fun toString(): String = "started"
}
)";
    auto source = parser.extract_function_infos(kotlin, Language::KOTLIN);
    assert(source.size() == 4);
    assert(!parser.last_extraction_has_errors());
    assert(parser.last_fun_interface_lines() == std::vector<int>{1});
    assert(source[0].name == "WhileSubscribed" && source[1].name == "WhileSubscribed");
    assert(source[0].start_line == 3 && source[1].start_line == 7);
    assert(source[0].signature != source[1].signature);
    std::string lexical = "// fun interface Fake\n/* outer /* fun interface Nested */ end */\nval a = \"fun interface String\"\nval b = \"\"\"fun interface Raw\nfun interface Raw2\"\"\"\nval c = 'f'\nval `fun interface Backtick` = 1\nfun /* nested /* x */ y */ interface Real {}\nfunction interface Wrong {}\n";
    auto adapted = kotlin_grammar_input(lexical);
    assert(adapted.fun_interface_lines == std::vector<int>{8});
    assert(adapted.text.size() == lexical.size());
    assert(adapted.text.substr(0, lexical.find("fun /* nested")) == lexical.substr(0, lexical.find("fun /* nested")));
    parser.extract_function_infos("fun broken( {", Language::KOTLIN);
    assert(parser.last_extraction_has_errors());
    parser.extract_function_infos("context() fun broken() {}", Language::KOTLIN);
    assert(parser.last_extraction_has_errors());
    assert(!parser.last_extraction_diagnostics().empty());

    auto local_source = parser.extract_function_infos(
        "fun collectWhile() { val collector = object { fun emit(value: Int) { predicate(value) } }; collect(collector) }", Language::KOTLIN);
    auto local_target = parser.extract_function_infos(
        "void drop() { class Collector { void emit(int value) { predicate(value); } }; }\n"
        "void collect_while() { class Collector { void emit(int value) { changed(value); } }; }", Language::CPP);
    assert(local_source.size() == 2 && local_target.size() == 4);
    assert(local_source[1].enclosing_functions == std::vector<std::string>{"collectWhile"});
    assert(local_target[1].enclosing_functions == std::vector<std::string>{"drop"});
    assert(!callable_owners_compatible(local_source[1], local_target[1]));
    assert(callable_owners_compatible(local_source[1], local_target[3]));

    const std::string macros = "void* invoke_suspend() {\n  coroutine_begin(this)\n  coroutine_yield(this, emit(value, this));\n  coroutine_end(this)\n  }\nvoid release_intercepted() { release(); }\n";
    auto macro_input = cpp_grammar_input(macros);
    assert(macro_input.text.size() == macros.size());
    assert(std::count(macro_input.text.begin(), macro_input.text.end(), '\n') == std::count(macros.begin(), macros.end(), '\n'));
    assert(macro_input.statement_macro_lines == (std::vector<int>{2, 4}));
    auto macro_tree = parser.parse_string(macros, Language::CPP);
    auto terminated_tree = parser.parse_string(macro_input.text, Language::CPP);
    assert(normalized_logic_tokens(macro_tree.get()) == normalized_logic_tokens(terminated_tree.get()));
    for (size_t i = 0; i < macros.size(); ++i)
        if (macros[i] != macro_input.text[i]) assert(std::isspace(static_cast<unsigned char>(macros[i])) && macros[i] != '\n' && macro_input.text[i] == ';');
    auto macro_functions = parser.extract_function_infos(macros, Language::CPP);
    assert(!parser.last_extraction_has_errors() && macro_functions.size() == 2);
    assert(parser.last_cpp_statement_macro_lines() == (std::vector<int>{2, 4}));
    assert(macro_functions[0].name == "invoke_suspend" && macro_functions[0].start_line == 1 && macro_functions[0].end_line == 5);
    assert(macro_functions[1].name == "release_intercepted" && macro_functions[1].start_line == 6);
    assert(macro_functions[0].identifiers.canonical_freq.contains("coroutinebegin"));
    assert(macro_functions[0].identifiers.canonical_freq.contains("coroutineyield"));
    assert(macro_functions[0].identifiers.canonical_freq.contains("coroutineend"));
    assert(!macro_functions[0].identifiers.canonical_freq.contains("releaseintercepted"));
    auto changed_macro_source = macros;
    changed_macro_source.replace(changed_macro_source.find("emit(value"), 4, "omit");
    auto changed_macro_functions = parser.extract_function_infos(changed_macro_source, Language::CPP);
    assert(!parser.last_extraction_has_errors());
    assert(normalized_logic_similarity(macro_functions[0].body_tree.get(), changed_macro_functions[0].body_tree.get()) < 1.0f);
    const std::string macro_lexical = R"literal(// coroutine_begin(this)
/* coroutine_end(this) */
#define coroutine_begin(c) ignored(c)
const char* text = "coroutine_begin(this)";
const char* raw = R"tag(coroutine_end(this)
 coroutine_begin(this))tag";
void work() { coroutine_begin(this); coroutine_end(this); }
int count = 1'000;
char delimiter = '\'';
)literal";
    assert(cpp_grammar_input(macro_lexical).text == macro_lexical);
    assert(cpp_grammar_input("void work() { unknown_begin(this)\n  finish(); }").statement_macro_lines.empty());
    assert(cpp_grammar_input("void work() { auto value = coroutine_begin(this); }").statement_macro_lines.empty());
    assert(cpp_grammar_input("void work() {\ncoroutine_end(this)\n}").statement_macro_lines.empty());
    parser.extract_function_infos("void work() { unknown_begin(this)\n  finish(); }", Language::CPP);
    assert(parser.last_extraction_has_errors());
    parser.extract_function_infos("void* broken() {\n  coroutine_begin(this)\n  return (;\n  }", Language::CPP);
    assert(parser.last_extraction_has_errors());

    ImportExtractor imports;
    auto empty = imports.extract_cpp_namespace("namespace kotlinx { namespace coroutines { namespace flow {} } }", "Empty.cpp");
    assert(empty.declared && !empty.ambiguous && empty.path == "kotlinx.coroutines.flow");
    auto helper_scope = imports.extract_cpp_namespace("namespace helper {} namespace intended { int work(){return 1;} }", "Mixed.cpp");
    assert(helper_scope.path == "intended" && !helper_scope.ambiguous);
    auto wrong_scope = imports.extract_cpp_namespace("namespace intended {} namespace wrong { int work(){return 1;} }", "Wrong.cpp");
    assert(wrong_scope.path == "wrong");
    auto mixed_scope = imports.extract_cpp_namespace("namespace intended { int work(){return 1;} } namespace wrong { int other(){return 2;} }", "Mixed.cpp");
    assert(mixed_scope.ambiguous);
    auto extension = parser.extract_function_infos("fun <T> Flow<T>.drop(count: Int): Flow<T> = this", Language::KOTLIN);
    assert(extension.size() == 1 && extension[0].extension_receiver == "Flow<T>");
    auto lowered = parser.extract_function_infos("namespace kotlinx::coroutines::flow { template<class T> std::shared_ptr<Flow<T>> drop(std::shared_ptr<Flow<T>> upstream, int count) { return upstream; } }", Language::CPP);
    assert(lowered.size() == 1 && lowered[0].is_namespace_function && lowered[0].first_parameter_type == "Flow");
    assert(callable_owners_compatible(extension[0], lowered[0]));
    auto runtime_type_filter = parser.extract_function_infos("fun <R> Flow<Any>.filterIsInstance(klass: KClass<R>): Flow<R> = this", Language::KOTLIN);
    auto reified_filter = parser.extract_function_infos("inline fun <reified R> Flow<Any>.filterIsInstance(): Flow<R> = this", Language::KOTLIN);
    auto pointer_filter = parser.extract_function_infos("namespace flow { template<class R, class T> Flow<R*> filter_is_instance(Flow<T*> upstream) { return {}; } }", Language::CPP);
    assert(runtime_type_filter[0].explicit_parameter_count == 1);
    assert(reified_filter[0].explicit_parameter_count == 0);
    assert(pointer_filter[0].explicit_parameter_count == 1);
    assert(!callable_owners_compatible(runtime_type_filter[0], pointer_filter[0]));
    assert(CodebaseComparator::compare_function_sets(runtime_type_filter, pointer_filter).matched_pairs == 0);
    assert(!callable_owners_compatible(pointer_filter[0], runtime_type_filter[0]));
    assert(callable_owners_compatible(reified_filter[0], pointer_filter[0]));
    auto runtime_lowered = parser.extract_function_infos("namespace flow { template<class R, class T> Flow<R*> filter_is_instance(Flow<T*> upstream, KClass<R> klass) { return {}; } }", Language::CPP);
    assert(callable_owners_compatible(runtime_type_filter[0], runtime_lowered[0]));
    assert(CodebaseComparator::compare_function_sets(runtime_type_filter, runtime_lowered).matched_pairs == 1);
    auto suspended_extension = parser.extract_function_infos("suspend fun Flow<Int>.process(value: Int = 1): Unit {}", Language::KOTLIN);
    auto continuation_lowered = parser.extract_function_infos("namespace flow { void* process(Flow<int> upstream, int value = 1, std::shared_ptr<Continuation<void*>> completion = nullptr) { return nullptr; } }", Language::CPP);
    assert(suspended_extension[0].explicit_parameter_count == 1 && suspended_extension[0].is_suspend_function);
    assert(continuation_lowered[0].explicit_parameter_count == 3 && continuation_lowered[0].trailing_continuation_parameter);
    assert(callable_owners_compatible(suspended_extension[0], continuation_lowered[0]));
    auto missing_value = parser.extract_function_infos("namespace flow { void* process(Flow<int> upstream, Continuation<void*>* completion) { return nullptr; } }", Language::CPP);
    assert(!callable_owners_compatible(suspended_extension[0], missing_value[0]));
    auto nested_callback = parser.extract_function_infos("namespace flow { void* process(Flow<int> upstream, std::function<void*(int, Continuation<void*>*)> value) { return nullptr; } }", Language::CPP);
    assert(nested_callback[0].explicit_parameter_count == 2 && !nested_callback[0].trailing_continuation_parameter);
    assert(callable_owners_compatible(suspended_extension[0], nested_callback[0]));
    auto explicit_continuation = parser.extract_function_infos("fun Flow<Int>.process(value: Continuation<Unit>): Unit {}", Language::KOTLIN);
    assert(!explicit_continuation[0].is_suspend_function);
    assert(callable_owners_compatible(explicit_continuation[0], missing_value[0]));
    auto non_suspend_factory = parser.extract_function_infos("fun Flow<Int>.process(value: suspend (Int) -> Unit): Unit {}", Language::KOTLIN);
    assert(!non_suspend_factory[0].is_suspend_function);
    assert(!callable_owners_compatible(non_suspend_factory[0], continuation_lowered[0]));
    auto zero_extension = parser.extract_function_infos("fun Flow<Int>.ready(): Boolean = true", Language::KOTLIN);
    auto no_parameters = parser.extract_function_infos("namespace flow { bool ready(void) { return true; } }", Language::CPP);
    assert(no_parameters[0].explicit_parameter_count == 0);
    assert(!callable_owners_compatible(zero_extension[0], no_parameters[0]));
    auto wrong_receiver = parser.extract_function_infos("namespace kotlinx::coroutines::flow { int drop(int count) { return count; } }", Language::CPP);
    assert(!callable_owners_compatible(extension[0], wrong_receiver[0]));
    auto wrong_wrapper = parser.extract_function_infos("namespace kotlinx::coroutines::flow { template<class T> int drop(Wrapper<Flow<T>> value) { return 0; } }", Language::CPP);
    assert(!callable_owners_compatible(extension[0], wrong_wrapper[0]));
    auto class_moved = parser.extract_function_infos("class Flow { int drop(int count) { return count; } };", Language::CPP);
    assert(!callable_owners_compatible(extension[0], class_moved[0]));
    auto qualified_receiver = parser.extract_function_infos("fun foo.Flow.drop(count: Int): foo.Flow = this", Language::KOTLIN);
    auto unrelated_receiver = parser.extract_function_infos("namespace intended { bar::Flow drop(bar::Flow value, int count) { return value; } }", Language::CPP);
    assert(!callable_owners_compatible(qualified_receiver[0], unrelated_receiver[0]));
    auto template_methods = parser.extract_function_infos("template<class T> struct Box { T map(T); }; template<class T> T Box<T>::map(T value) { return value; }", Language::CPP);
    auto kotlin_method = parser.extract_function_infos("class Box<T> { fun map(value: T): T = value }", Language::KOTLIN);
    assert(template_methods.size() == 1 && template_methods[0].name == "map");
    assert(callable_owners_compatible(kotlin_method[0], template_methods[0]));
    auto specialized = parser.extract_function_infos("template<class T> T identity(T input) { return input; } template<> int identity<int>(int input) { return input; }", Language::CPP);
    assert(specialized.size() == 2 && specialized[0].name == "identity" && specialized[1].name == "identity");

    auto review = review_cpp(R"(
enum class HolderType { VALUE, FAILED, CLOSED };
struct StateMarker { const char* name; };
struct Empty {};
class Destructor { public: virtual ~Destructor() = default; };
class Interface { public: virtual void run() = 0; virtual ~Interface() = default; };
class Marker : public Interface { public: ~Marker() = default; };
)");
    assert(review.types.size() == 5);
    assert(review.types[0].category == "implemented_type");
    assert(review.types[1].category == "empty_type_review");
    assert(review.types[2].category == "destructor_only_review");
    assert(review.types[3].category == "interface");
    assert(review.types[4].category == "inherited_marker_review");
    assert(review_cpp("void pending();").file_category == "declarations_only_review");
    assert(review_cpp("extern int state;").file_category == "declarations_only_review");
    assert(review_cpp("int state = 1;").file_category.empty());
    assert(review_cpp("int capacity() { return 64; }").file_category.empty());
    auto instantiation = review_cpp("template class AbstractCoroutine<Unit>;");
    assert(instantiation.has_implementation);
    assert(instantiation.file_category == "parse_error_review");
    assert(review_cpp("#include \"Flow.hpp\"\nnamespace n {}\n").file_category == "implementation_not_in_translation_unit");
    assert(review_cpp("struct Broken { @").file_category == "parse_error_review");

    auto dir = std::filesystem::temp_directory_path() / ("ast-extraction-review-" + std::to_string(getpid()));
    std::filesystem::create_directories(dir / "source");
    std::filesystem::create_directories(dir / "target");
    for (int i = 0; i < 25; ++i) { std::ofstream f(dir / "target" / ("Empty" + std::to_string(i) + ".cpp")); f << "// no implementation\n"; }
    { std::ofstream f(dir / "target" / "Quoted.hpp"); f << "struct EmptyType {};"; }
    { std::ofstream f(dir / "target" / "Inline.hpp"); f << "struct Inline { int value() { return 1; } };"; }
    { std::ofstream f(dir / "target" / "Inline.cpp"); f << "#include \"Inline.hpp\"\n"; }
    SymbolAnalysisOptions options; options.stubs = true; options.json = true;
    std::ostringstream captured;
    auto* old = std::cout.rdbuf(captured.rdbuf());
    cmd_symbols((dir / "source").string(), (dir / "target").string(), options);
    std::cout.rdbuf(old);
    assert(captured.str().find("Empty24.cpp") != std::string::npos);
    assert(captured.str().find("review_candidates_not_verified_stubs") != std::string::npos);
    assert(captured.str().find("\"file\":\"Inline.cpp\"") == std::string::npos);
    assert(captured.str().find("\"translation_unit\":\"Inline.cpp\"") != std::string::npos);
    auto json = captured.str();
    captured.str(""); captured.clear(); options.json = false;
    old = std::cout.rdbuf(captured.rdbuf());
    cmd_symbols((dir / "source").string(), (dir / "target").string(), options);
    std::cout.rdbuf(old);
    assert(captured.str().find("Empty24.cpp") != std::string::npos);
    assert(captured.str().find("... and") == std::string::npos);
    if (argc == 3) {
        auto kt = parser.extract_function_infos_from_file(argv[1], Language::KOTLIN);
        assert(kt.size() == 10);
        std::vector<int> factories;
        for (const auto& f : kt) if (f.name == "WhileSubscribed") factories.push_back(f.start_line);
        assert(factories.size() == 2 && factories[0] != factories[1]);
        auto target = parser.extract_function_infos_from_file(argv[2], Language::CPP);
        int commands = 0, strings = 0;
        for (const auto& f : target) { commands += f.name == "command"; strings += f.name == "to_string"; assert(f.name != "<anonymous>"); }
        assert(commands == 3 && strings == 3);
        std::cout << "Real SharingStarted inventory: " << kt.size() << " Kotlin, " << target.size() << " C++; two distinct factory signatures, three command/to_string definitions\n";
    }
    std::filesystem::remove_all(dir);
    std::cout << "Callable extraction, grammar compatibility, classifier and full report assertions passed\n";
}
