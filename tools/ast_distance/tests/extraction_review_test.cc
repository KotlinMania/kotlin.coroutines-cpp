#include "ast_parser.hpp"
#include "cpp_review.hpp"
#include "symbol_analysis.hpp"
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
