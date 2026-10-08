#include "ast_parser.hpp"
#include "similarity.hpp"
#include "callable_identity.hpp"
#include <cassert>
#include <iostream>
using namespace ast_distance;
struct Score { float combined, logic; };
Score score(ASTParser& parser, const std::string& source, const std::string& target) {
    auto kotlin = parser.extract_function_infos(source, Language::KOTLIN);
    assert(!parser.last_extraction_has_errors() && kotlin.size() == 1);
    auto cpp = parser.extract_function_infos(target, Language::CPP);
    assert(!parser.last_extraction_has_errors() && cpp.size() == 1);
    return {ASTSimilarity::function_parameter_body_cosine_similarity(kotlin[0].body_tree.get(), cpp[0].body_tree.get(), kotlin[0].identifiers, cpp[0].identifiers),
        normalized_logic_similarity(kotlin[0].body_tree.get(), cpp[0].body_tree.get())};
}
std::string replace_once(std::string text, const std::string& from, const std::string& to) {
    auto pos = text.find(from); assert(pos != std::string::npos); text.replace(pos, from.size(), to); return text;
}
int main() {
    ASTParser parser;
    FunctionInfo foo, bar, qualified;
    foo.qualified_name = "Foo::command"; bar.qualified_name = "Bar::command";
    qualified.qualified_name = "ns::Foo::command";
    assert(!callable_owners_compatible(foo, bar));
    assert(callable_owners_compatible(foo, qualified));
    qualified.qualified_name = "Foo.Companion::command";
    assert(callable_owners_compatible(foo, qualified));
    const std::string source = "fun compareValue(inputValue: Int, limitValue: Int): Int { if (inputValue > limitValue) { return inputValue + 50 } else { return limitValue - 1 } }";
    const std::string faithful = "int compare_value(int input_value, int limit_value) { if (input_value > limit_value) { return input_value + 50; } else { return limit_value - 1; } }";
    auto baseline = score(parser, source, faithful);
    std::cout << "faithful branch combined=" << baseline.combined << " logic=" << baseline.logic << '\n';
    assert(baseline.combined > 0.85f);
    for (const auto& [label, variant] : std::vector<std::pair<std::string, std::string>>{
        {"comparison polarity", replace_once(faithful, ">", "<")},
        {"arithmetic", replace_once(faithful, "+ 50", "- 50")},
        {"literal timeout", replace_once(faithful, "50", "500")},
        {"omitted branch", replace_once(faithful, " else { return limit_value - 1; }", "")},
        {"parameter type", replace_once(faithful, "int input_value", "double input_value")},
        {"parameter omitted", replace_once(faithful, ", int limit_value", "")}}) {
        auto changed = score(parser, source, variant);
        std::cout << label << " combined=" << changed.combined << " logic=" << changed.logic << '\n';
        assert(changed.combined < baseline.combined - 0.01f);
        assert(changed.logic < baseline.logic);
    }
    const std::string vocab = "// inputValue limitValue compareValue return if else next add remove\n";
    auto comments = score(parser, source, vocab + faithful + "/* inputValue limitValue */");
    assert(std::abs(comments.combined - baseline.combined) < 0.000001f);
    for (const auto& fake : std::vector<std::string>{
        "int compare_value(int input_value, int limit_value) { const char* words = \"inputValue limitValue compareValue if return else 50 1\"; return 0; }",
        "int compare_value(int input_value, int limit_value) { int copied_input = input_value; int copied_limit = limit_value; int copied_more = input_value + 50; int copied_less = limit_value - 1; return 0; }"}) {
        auto stuffed = score(parser, source, vocab + fake);
        std::cout << "vocabulary-stuffed fake combined=" << stuffed.combined << " logic=" << stuffed.logic << '\n';
        assert(stuffed.combined < 0.85f && stuffed.combined < baseline.combined - 0.2f);
    }
    auto plain = score(parser, "fun value(x: Int): Int { return x + 1 }", "int value(int x) { return x + 1; }");
    auto renamed = score(parser, "fun value(x: Int): Int { return x + 1 }", "int value(int y) { return y + 1; }");
    assert(renamed.combined < plain.combined - 0.2f);
    auto call = score(parser, "fun advance(cursor: Cursor): Int { return cursor.next() }", "int advance(Cursor cursor) { return cursor.next(); }");
    auto changed_call = score(parser, "fun advance(cursor: Cursor): Int { return cursor.next() }", "int advance(Cursor cursor) { return cursor.remove(); }");
    assert(changed_call.combined < call.combined - 0.05f);
    auto add_ids = parser.extract_function_infos("fun apply(values: Values) { values.add(1) }", Language::KOTLIN)[0].identifiers;
    assert(add_ids.canonical_freq.contains("add") && add_ids.canonical_freq.contains("values"));
    auto next_ids = parser.extract_function_infos("fun apply(values: Values) { values.next() }", Language::KOTLIN)[0].identifiers;
    assert(next_ids.canonical_freq.contains("next"));
    auto loop = score(parser, "fun sumValue(limit: Int): Int { var value = 0; while (value < limit) { value += 1 }; return value }",
        "int sum_value(int limit) { int value = 0; while (value < limit) { value += 1; } return value; }");
    auto drift_loop = score(parser, "fun sumValue(limit: Int): Int { var value = 0; while (value < limit) { value += 1 }; return value }",
        "int sum_value(int limit) { int value = 0; while (value > limit) { value += 1; } return value; }");
    std::cout << "faithful loop combined=" << loop.combined << " changed=" << drift_loop.combined << '\n';
    assert(loop.combined > 0.95f && loop.logic == 1.0f && drift_loop.combined < loop.combined - 0.01f);
    auto string_source = "fun state(): String { return \"ready\" }";
    auto string_same = score(parser, string_source, "const char* state() { return \"ready\"; }");
    auto string_changed = score(parser, string_source, "const char* state() { return \"failed\"; }");
    assert(string_changed.combined < string_same.combined - 0.1f);
    auto unsigned_parameter = score(parser, "fun count(value: UInt): UInt { return value }", "unsigned int count(unsigned int value) { return value; }");
    assert(unsigned_parameter.logic == 1.0f);
    auto signed_parameter = score(parser, "fun count(value: UInt): UInt { return value }", "unsigned int count(int value) { return value; }");
    assert(signed_parameter.combined < unsigned_parameter.combined);
    auto expression = score(parser, "fun state(): String = \"ready\"", "const char* state() { return \"ready\"; }");
    assert(expression.logic == 1.0f && expression.combined > 0.95f);
    auto generic = score(parser, "fun identity(values: Box<Int>): Box<Int> = values", "Box<int> identity(Box<int> values) { return values; }");
    assert(generic.logic == 1.0f);
    auto control = parser.extract_function_infos("fun broken(x: Int): Int { if (x < 0) { throw Error() }; while (true) { if (x == 0) break; continue }; return x }", Language::KOTLIN);
    assert(control.size() == 1);
    auto tokens = normalized_logic_tokens(control[0].body_tree.get());
    for (auto type : {NodeType::THROW, NodeType::BREAK, NodeType::CONTINUE, NodeType::RETURN})
        assert(std::find(tokens.begin(), tokens.end(), "node:" + std::to_string(static_cast<int>(type))) != tokens.end());
    assert(normalized_number("1_000L") == normalized_number("1000LL"));
    assert(normalized_number("0x3e8") == "1000");
    assert(normalized_number("077") == "63");
    assert(normalized_number("1'000") == "1000");
    for (const auto& expression : {std::string("++x"), std::string("x++"), std::string("--x"), std::string("x--")}) {
        auto source_update = "fun update(x: Int): Int { return " + expression + " }";
        auto target_update = "int update(int x) { return " + expression + "; }";
        auto same_update = score(parser, source_update, target_update);
        auto operation = expression.find("++") != std::string::npos ? "++" : "--";
        auto reversed_update = score(parser, source_update,
            replace_once(target_update, operation, std::string(operation) == "++" ? "--" : "++"));
        auto removed_update = score(parser, source_update, "int update(int x) { return x; }");
        assert(same_update.logic == 1.0f);
        assert(reversed_update.logic < same_update.logic && reversed_update.combined < same_update.combined);
        assert(removed_update.logic < same_update.logic && removed_update.combined < same_update.combined);
    }
    auto prefix_update = score(parser, "fun update(x: Int): Int { return ++x }", "int update(int x) { return ++x; }");
    auto postfix_drift = score(parser, "fun update(x: Int): Int { return ++x }", "int update(int x) { return x++; }");
    assert(postfix_drift.logic < prefix_update.logic);
    Tree first(static_cast<int>(NodeType::NUMBER), "50"), second(static_cast<int>(NodeType::NUMBER), "500");
    IdentifierStats empty;
    assert(ASTSimilarity::function_parameter_body_cosine_similarity(&first, &second, empty, empty) < 0.5f);
    std::cout << "Normalized branch/loop spelling and operator/literal/call/parameter/control-flow drift assertions passed\n";
}
