#include "transliteration_engine.hpp"
#include <cassert>
#include <iostream>
using namespace ast_distance;
static std::string changed(std::string input, const std::string& before, const std::string& after) {
    auto position = input.find(before); assert(position != std::string::npos);
    input.replace(position, before.size(), after); return input;
}
int main() {
    const std::string source = "/** Returns inputValue after the limit check. */\nfun compareValue(inputValue: Int, limitValue: Int): Int { if (inputValue > limitValue) { return inputValue + 50 } else { return limitValue - 1 } }";
    auto output = transliterate(source, Language::KOTLIN, Language::CPP);
    std::cout << output.buffer;
    assert(output.rule_misses == 0 && output.rule_coverage == 1);
    assert(output.buffer.find("/** Returns inputValue after the limit check. */") != std::string::npos);
    size_t previous = 0;
    for (const auto& span : output.spans) {
        assert(span.source_start <= span.source_end && span.source_end <= source.size());
        assert(span.target_start >= previous && span.target_end <= output.buffer.size());
        previous = span.target_end;
        assert(span.supported);
    }
    auto baseline = transliteration_distance(source, Language::KOTLIN, output.buffer, Language::CPP);
    std::cout << "faithful=" << baseline.score << " logic=" << baseline.normalized_logic << '\n';
    assert(!baseline.translated_parse_errors && !baseline.target_parse_errors);
    assert(baseline.score > .99f && baseline.normalized_logic == 1 && baseline.documentation_parity == 1);
    for (const auto& variant : {changed(output.buffer, ">", "<"), changed(output.buffer, "+ 50", "- 50"), changed(output.buffer, "50", "500")}) {
        auto report = transliteration_distance(source, Language::KOTLIN, variant, Language::CPP);
        assert(report.score < baseline.score && report.normalized_logic < 1);
    }
    // These bodies have identical token frequencies but different execution order.
    const std::string ordered_source = "fun orderedValue(): Int { first(); second(); return 1 }";
    auto ordered_output = transliterate(ordered_source, Language::KOTLIN, Language::CPP);
    auto ordered = transliteration_distance(ordered_source, Language::KOTLIN, ordered_output.buffer, Language::CPP);
    auto swapped = transliteration_distance(ordered_source, Language::KOTLIN,
        changed(ordered_output.buffer, "first(); second();", "second(); first();"), Language::CPP);
    assert(ordered.score == 1 && swapped.score < ordered.score);
    assert(std::abs(swapped.score - 15.0f / 17.0f) < 0.000001f);
    assert(swapped.translated_ast_cosine == ordered.translated_ast_cosine);
    assert(swapped.score == swapped.translated_text_cosine);
    auto whitespace = transliteration_distance(ordered_source, Language::KOTLIN,
        changed(ordered_output.buffer, "first(); second();", "first ( ) ;\n second ( ) ;"), Language::CPP);
    assert(whitespace.score == 1);
    auto renamed = transliteration_distance(source, Language::KOTLIN,
        changed(output.buffer, "input_value", "inputValue"), Language::CPP);
    assert(renamed.score < baseline.score); // Exact target spelling, no identifier canonicalization.
    const std::string string_source = "fun literalValue(): String = \"red blue\"";
    auto string_output = transliterate(string_source, Language::KOTLIN, Language::CPP);
    auto string_changed = transliteration_distance(string_source, Language::KOTLIN,
        changed(string_output.buffer, "red blue", "blue red"), Language::CPP);
    assert(string_changed.score < 1); // Literal contents stay intact as one token.
    auto doc_order = transliteration_distance(source, Language::KOTLIN,
        changed(output.buffer, "after the limit", "the after limit"), Language::CPP);
    assert(doc_order.documentation_parity < 1 && doc_order.score == baseline.score);
    auto documentation_removed = output.buffer.substr(output.buffer.find("int compare_value"));
    auto missing_docs = transliteration_distance(source, Language::KOTLIN, documentation_removed, Language::CPP);
    assert(missing_docs.score == baseline.score && missing_docs.documentation_parity == 0);
    auto extra_docs = transliteration_distance(source, Language::KOTLIN, output.buffer + "\n// inputValue limitValue if return else add remove", Language::CPP);
    assert(extra_docs.score == baseline.score && extra_docs.documentation_parity < 1);
    const std::string fake = "/** Returns inputValue after the limit check. */\nint compare_value(int input_value, int limit_value) { const char* words = \"inputValue limitValue compareValue if return else 50 1\"; return 0; }";
    auto stuffed = transliteration_distance(source, Language::KOTLIN, fake, Language::CPP);
    assert(stuffed.score < .6f && stuffed.score < baseline.score - .25f);
    const std::string documented = "/** Keeps the narrative inputValue unchanged.\n * @param inputValue the input\n * @see Helper.nextValue\n * Uses [inputValue], [Helper.nextValue] and [next item][Helper.nextValue].\n */\nfun nextValue(inputValue: Int): Int = inputValue + 1";
    auto documented_source = documented;
    auto documented_output = transliterate(documented_source, Language::KOTLIN, Language::CPP);
    assert(documented_output.rule_misses == 0 && documented_output.documentation_misses == 0);
    assert(documented_output.buffer.find("narrative inputValue unchanged") != std::string::npos);
    assert(documented_output.buffer.find("@param input_value") != std::string::npos);
    assert(documented_output.buffer.find("@see Helper::next_value") != std::string::npos);
    assert(documented_output.buffer.find("\\ref input_value") != std::string::npos);
    assert(documented_output.buffer.find("\\ref Helper::next_value \"next item\"") != std::string::npos);
    auto doc_faithful = transliteration_distance(documented_source, Language::KOTLIN, documented_output.buffer, Language::CPP);
    assert(doc_faithful.documentation_parity == 1 && doc_faithful.score > .99f);
    auto doc_changed = transliteration_distance(documented_source, Language::KOTLIN, changed(documented_output.buffer, "the input", "the unrelated output"), Language::CPP);
    assert(doc_changed.documentation_parity < 1 && doc_changed.score == doc_faithful.score);
    const std::string primitive_docs = "/** Accepts [Int], returns [kotlin.String], and sees [Helper]. @see kotlin.Int Also [a wide value][Long]. */\nfun primitiveValue(inputValue: kotlin.Int): kotlin.String = \"ready\"";
    auto primitive_output = transliterate(primitive_docs, Language::KOTLIN, Language::CPP);
    assert(primitive_output.rule_misses == 0);
    assert(primitive_output.buffer.find("\\c int") != std::string::npos);
    assert(primitive_output.buffer.find("\\c std::string") != std::string::npos);
    assert(primitive_output.buffer.find("@see \\c int") != std::string::npos);
    assert(primitive_output.buffer.find("`long long` (a wide value)") != std::string::npos);
    assert(primitive_output.buffer.find("\\ref Helper") != std::string::npos);
    assert(primitive_output.buffer.find("std::string primitive_value(int input_value)") != std::string::npos);
    auto primitive_faithful = transliteration_distance(primitive_docs, Language::KOTLIN, primitive_output.buffer, Language::CPP);
    assert(primitive_faithful.documentation_parity == 1 && primitive_faithful.score > .99f);
    auto primitive_markup = transliteration_distance(primitive_docs, Language::KOTLIN, changed(primitive_output.buffer, "\\c int", "`int`"), Language::CPP);
    assert(primitive_markup.documentation_parity == 1 && primitive_markup.score == primitive_faithful.score);
    auto primitive_wrong = transliteration_distance(primitive_docs, Language::KOTLIN, changed(primitive_output.buffer, "\\c int", "\\c double"), Language::CPP);
    assert(primitive_wrong.documentation_parity < 1 && primitive_wrong.score == primitive_faithful.score);
    auto capital_parameter = transliterate("/** @param String the value */\nfun capitalValue(String: Int): Int = String", Language::KOTLIN, Language::CPP);
    assert(capital_parameter.buffer.find("@param string the value") != std::string::npos);
    auto fenced = transliterate("/** Example:\n * ```kotlin\n * fun example() = 1\n * ```\n */\nfun exampleValue(): Int = 1", Language::KOTLIN, Language::CPP);
    assert(fenced.documentation_misses == 1 && fenced.rule_misses == 0 && fenced.rule_coverage == 1);
    assert(fenced.buffer.find("fun example() = 1") != std::string::npos && !fenced.diagnostics.empty());
    auto markdown = transliterate("/** See [guide](https://example.test/doc) and [Companion]. */\nfun guideValue(): Int = 1", Language::KOTLIN, Language::CPP);
    assert(markdown.buffer.find("[guide](https://example.test/doc)") != std::string::npos);
    assert(markdown.buffer.find("\\ref Companion") != std::string::npos);
    auto literal_refs = transliterate("fun refValue(): String = \"[inputValue]\"", Language::KOTLIN, Language::CPP);
    assert(literal_refs.buffer.find("\"[inputValue]\"") != std::string::npos);
    const std::string overloads = "fun applyValue(x: Int): Int = x + 1\nfun applyValue(x: Int, y: Int): Int = x + y";
    auto over = transliterate(overloads, Language::KOTLIN, Language::CPP);
    auto over_score = transliteration_distance(overloads, Language::KOTLIN, over.buffer, Language::CPP);
    assert(over_score.normalized_logic == 1 && over_score.symbol_parity == 1);
    auto reversed_over = over.buffer.substr(over.buffer.find('\n') + 1) + over.buffer.substr(0, over.buffer.find('\n') + 1);
    auto reordered = transliteration_distance(overloads, Language::KOTLIN, reversed_over, Language::CPP);
    assert(reordered.normalized_logic == 1 && reordered.functions.size() == 2);
    assert(reordered.score < over_score.score);
    assert(reordered.functions[0].target_line != reordered.functions[1].target_line);
    auto lost = transliteration_distance(overloads, Language::KOTLIN, "int apply_value(int x) { return x + 1; }", Language::CPP);
    assert(lost.symbol_parity == .5f && lost.normalized_logic <= .5f);
    auto metadata = transliteration_distance("fun plainValue(): Int = 1", Language::KOTLIN,
        "// Transliterated from: original/path.kt:1-2\n// port-lint: source=original/path.kt\nint plain_value() { return 1; }", Language::CPP);
    assert(metadata.documentation_parity == 1 && metadata.normalized_logic == 1);
    auto narrative_extra = transliteration_distance("fun plainValue(): Int = 1", Language::KOTLIN,
        "// A copied narrative about variables and algorithms\nint plain_value() { return 1; }", Language::CPP);
    assert(narrative_extra.documentation_parity == 0 && narrative_extra.score == metadata.score);
    auto expression = transliterate("fun state(): String = \"ready\"", Language::KOTLIN, Language::CPP);
    assert(expression.rule_misses == 0 && expression.buffer.find("return \"ready\"") != std::string::npos);
    auto loop = transliterate("fun sumValue(limit: Int): Int { var value = 0; while (value < limit) { value += 1 }; val total = value; return total }", Language::KOTLIN, Language::CPP);
    std::cout << loop.buffer;
    assert(loop.rule_misses == 0 && loop.buffer.find("const auto total") != std::string::npos);
    auto loop_score = transliteration_distance("fun sumValue(limit: Int): Int { var value = 0; while (value < limit) { value += 1 }; val total = value; return total }", Language::KOTLIN, loop.buffer, Language::CPP);
    assert(loop_score.score > .99f && !loop_score.translated_parse_errors);
    auto inline_comment = transliterate("fun commentValue(): Int { // keep this comment\n return 5 }", Language::KOTLIN, Language::CPP);
    assert(inline_comment.rule_misses == 0 && inline_comment.buffer.find("// keep this comment\n") != std::string::npos);
    auto comment_score = transliteration_distance("fun commentValue(): Int { // keep this comment\n return 5 }", Language::KOTLIN, inline_comment.buffer, Language::CPP);
    assert(!comment_score.translated_parse_errors && comment_score.normalized_logic == 1);
    auto packaged = transliterate("package original.flow\nfun packagedValue(): Int = 1", Language::KOTLIN, Language::CPP);
    assert(packaged.rule_misses == 0 && packaged.buffer.find("namespace original::flow {") != std::string::npos);
    auto packaged_score = transliteration_distance("package original.flow\nfun packagedValue(): Int = 1", Language::KOTLIN, packaged.buffer, Language::CPP);
    assert(packaged_score.score > .99f && !packaged_score.translated_parse_errors);
    auto defaults = transliterate("fun addValue(x: Int = 5): Int = x + 1", Language::KOTLIN, Language::CPP);
    assert(defaults.rule_misses == 0 && defaults.buffer.find("int x = 5") != std::string::npos);
    for (const std::string visibility : {"public", "internal", "private"}) {
        const std::string visible_source = visibility +
            " fun checkValue(input: Int): Int { if (input > 3) { return input + 1 } else { return 0 } }";
        auto visible = transliterate(visible_source, Language::KOTLIN, Language::CPP);
        assert(visible.rule_misses == 0 && visible.rule_coverage == 1);
        assert(visible.buffer.starts_with(visibility == "private" ? "static int" : "int"));
        auto faithful = transliteration_distance(visible_source, Language::KOTLIN, visible.buffer, Language::CPP);
        assert(faithful.normalized_logic == 1 && !faithful.translated_parse_errors);
        auto drift = transliteration_distance(visible_source, Language::KOTLIN,
            changed(visible.buffer, "> 3", "< 3"), Language::CPP);
        assert(drift.normalized_logic < faithful.normalized_logic && drift.score < faithful.score);
    }
    for (const auto& modifier : {"internal suspend", "private inline", "protected"}) {
        auto unsupported_modifier = transliterate(std::string(modifier) + " fun value(): Int = 1",
            Language::KOTLIN, Language::CPP);
        assert(unsupported_modifier.rule_misses > 0 && !unsupported_modifier.diagnostics.empty());
    }
    for (const auto& unsupported : {"suspend fun awaitValue(): Int = 1", "fun nullable(): Int? = null", "fun <T> identity(x: T): T = x", "fun String.extension(): Int = 1", "fun callback(): () -> Int = { 1 }", "class Hidden(val value: Int) { fun run(): Int = value }", "class Child : Parent() { fun run(): Int = 1 }", "fun rangeValue(): Int { for (i in 0 until 5) { next(i) }; return 1 }", "fun inferred() = 1", "fun named(): Int = next(value = 1)", "fun varargValue(vararg values: Int): Int = 1", "fun localNullable(): Int { val value: Int? = null; return 1 }"}) {
        auto fallback = transliterate(unsupported, Language::KOTLIN, Language::CPP);
        assert(fallback.rule_misses > 0 && fallback.rule_coverage == 0 && !fallback.diagnostics.empty());
        assert(fallback.buffer.find("__ast_distance_unmapped__") != std::string::npos);
        auto report = transliteration_distance(unsupported, Language::KOTLIN, fallback.buffer, Language::CPP);
        assert(report.score < 1 && report.score == report.translated_text_cosine);
        assert(report.fallback_penalty == 1 && report.translation.rule_misses > 0);
    }
    const std::string member_source = "class Counter {\n"
        " /** Returns the adjusted [inputValue]. */\n"
        " fun nextValue(inputValue: Int): Int = inputValue + 1\n"
        " private fun checkValue(inputValue: Int): Int = inputValue - 1\n"
        " fun nextValue(inputValue: Int, offsetValue: Int): Int = inputValue + offsetValue\n}\n"
        "class Other { fun nextValue(inputValue: Int): Int = inputValue + 9 }";
    const auto member_output = transliterate(member_source, Language::KOTLIN, Language::CPP);
    assert(member_output.rule_misses == 0 && member_output.rule_coverage == 1);
    assert(member_output.buffer.find("class Counter final") != std::string::npos);
    assert(member_output.buffer.find("private:\nint check_value") != std::string::npos);
    const auto member_report = transliteration_distance(member_source, Language::KOTLIN, member_output.buffer, Language::CPP);
    assert(!member_report.translated_parse_errors && !member_report.target_parse_errors);
    assert(member_report.functions.size() == 4 && member_report.normalized_logic == 1);
    assert(member_report.score == 1 && member_report.documentation_parity == 1);
    const auto member_drift = transliteration_distance(member_source, Language::KOTLIN,
        changed(member_output.buffer, "input_value + 1", "input_value - 1"), Language::CPP);
    assert(member_drift.normalized_logic < 1 && member_drift.score < 1);
    const auto lost_member = transliteration_distance(member_source, Language::KOTLIN,
        changed(member_output.buffer, "next_value(int input_value, int offset_value)", "different_value(int input_value, int offset_value)"), Language::CPP);
    assert(lost_member.functions.size() == 3 && lost_member.symbol_parity == .75f);
    for (const std::string unsupported_class : {
        "public interface Contract { fun run(): Int }",
        "private class Hidden { fun run(): Int = 1 }",
        "class WithField { val value: Int = 1; fun run(): Int = value }",
        "class `odd name` { fun run(): Int = 1 }"}) {
        const auto class_output = transliterate(unsupported_class, Language::KOTLIN, Language::CPP);
        assert(class_output.rule_misses > 0 && class_output.rule_coverage == 0);
        assert(class_output.buffer.find("class_declaration") != std::string::npos);
    }
    const std::string partial_class = "class Partial { "
        "suspend fun nextValue(inputValue: Int): Int = inputValue + 1; "
        "fun nextValue(inputValue: Int, offsetValue: Int): Int = inputValue + offsetValue }";
    const auto partial_output = transliterate(partial_class, Language::KOTLIN, Language::CPP);
    assert(partial_output.rule_misses > 0 && partial_output.rule_coverage == 0);
    const auto partial_report = transliteration_distance(partial_class, Language::KOTLIN,
        "class Partial final { public: int next_value(int input_value) { return input_value + 1; } "
        "int next_value(int input_value, int offset_value) { return input_value + offset_value; } };", Language::CPP);
    assert(partial_report.normalized_logic == 0 && partial_report.functions.size() == 2);
    for (const auto& method : partial_report.functions) assert(method.emitted_tokens.empty());
    bool rejected = false;
    try { (void)transliterate("int foo(){return 1;}", Language::CPP, Language::KOTLIN); }
    catch (const std::runtime_error&) { rejected = true; }
    assert(rejected);
    std::cout << "AST emission, original spans, overloads, drift, documentation and explicit fallbacks passed\n";
}
