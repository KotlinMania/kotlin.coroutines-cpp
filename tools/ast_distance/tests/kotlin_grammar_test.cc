#include <tree_sitter/api.h>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "kotlin_grammar_compat.hpp"

extern "C" const TSLanguage* tree_sitter_kotlin();

static void collect(TSNode node, const char* type, std::vector<TSNode>& nodes) {
    if (std::strcmp(ts_node_type(node), type) == 0) nodes.push_back(node);
    for (uint32_t i = 0; i < ts_node_child_count(node); ++i)
        collect(ts_node_child(node, i), type, nodes);
}

int main() {
    TSParser* parser = ts_parser_new();
    assert(ts_parser_set_language(parser, tree_sitter_kotlin()));
    // The original delimiters, operators and UTF-8 byte positions must survive.
    const std::string source =
        "// λ source positions\nfun bindings() {\n"
        " for ([index, field] in boundFields.withIndex()) { use(index, field) }\n"
        " for ((index, parameter,) in parameters.withIndex()) { use(index, parameter) }\n"
        " val [first, second,] = pair\n"
        " val (renamed = original, other) = named\n"
        " val open = (0..<fields.lastIndex).map { index -> fields[index] }\n"
        " val closed = 0..fields.lastIndex\n}\n";
    TSTree* tree = ts_parser_parse_string(parser, nullptr, source.data(), source.size());
    TSNode root = ts_tree_root_node(tree);
    if (ts_node_has_error(root)) {
        char* diagnostic = ts_node_string(root);
        std::cerr << diagnostic << '\n';
        std::free(diagnostic);
    }
    assert(!ts_node_has_error(root));
    assert(ast_distance::kotlin_identifier_diagnostics(root, source).empty());
    std::vector<TSNode> declarations, ranges;
    collect(root, "multi_variable_declaration", declarations);
    collect(root, "range_expression", ranges);
    assert(declarations.size() == 4 && ranges.size() == 2);
    const std::vector<std::string> spellings = {
        "[index, field]", "(index, parameter,)", "[first, second,]",
        "(renamed = original, other)"};
    for (size_t i = 0; i < spellings.size(); ++i) {
        const uint32_t begin = ts_node_start_byte(declarations[i]);
        const uint32_t end = ts_node_end_byte(declarations[i]);
        assert(source.substr(begin, end - begin) == spellings[i]);
        assert(begin == source.find(spellings[i]));
        assert(ts_node_start_point(declarations[i]).row == i + 2);
        assert(ts_node_type(ts_node_child(declarations[i], 0))[0] == spellings[i][0]);
    }
    assert(std::strcmp(ts_node_type(ts_node_child(ranges[0], 1)), "..<") == 0);
    assert(std::strcmp(ts_node_type(ts_node_child(ranges[1], 1)), "..") == 0);
    ts_tree_delete(tree);
    const std::string full =
        "fun fullBindings() {\n"
        " (val function, val reference, val samType = samConversionType, val referenceType) = expression.parseAdaptedBlock() ?: return null;\n"
        " [val first: Int, var second,] = pair\n}\n";
    tree = ts_parser_parse_string(parser, nullptr, full.data(), full.size());
    if (ts_node_has_error(ts_tree_root_node(tree))) {
        char* diagnostic = ts_node_string(ts_tree_root_node(tree));
        std::cerr << diagnostic << '\n';
        std::free(diagnostic);
    }
    assert(!ts_node_has_error(ts_tree_root_node(tree)));
    std::vector<TSNode> full_declarations;
    collect(ts_tree_root_node(tree), "destructuring_declaration", full_declarations);
    assert(full_declarations.size() == 2);
    assert(full.substr(ts_node_start_byte(full_declarations[0]),
        ts_node_end_byte(full_declarations[0]) - ts_node_start_byte(full_declarations[0])) ==
        "(val function, val reference, val samType = samConversionType, val referenceType) = expression.parseAdaptedBlock() ?: return null");
    ts_tree_delete(tree);
    const std::string contexts =
        "// λ context positions\n"
        "context(context: CheckerContext, reporter: DiagnosticReporter,)\n"
        "override fun check(expression: Expression) { use(context, reporter) }\n"
        "context(owner: Owner) private fun scope(): Boolean = true\n";
    tree = ts_parser_parse_string(parser, nullptr, contexts.data(), contexts.size());
    assert(!ts_node_has_error(ts_tree_root_node(tree)));
    std::vector<TSNode> context_lists, context_functions;
    collect(ts_tree_root_node(tree), "context_parameters", context_lists);
    collect(ts_tree_root_node(tree), "function_declaration", context_functions);
    assert(context_lists.size() == 2 && context_functions.size() == 2);
    assert(ts_node_start_byte(context_lists[0]) == contexts.find("context(context:"));
    assert(contexts.substr(ts_node_start_byte(context_lists[0]),
        ts_node_end_byte(context_lists[0]) - ts_node_start_byte(context_lists[0])) ==
        "context(context: CheckerContext, reporter: DiagnosticReporter,)");
    assert(ts_node_start_point(context_lists[0]).row == 1);
    ts_tree_delete(tree);
    // A context list introduces a soft keyword, not a reserved identifier.
    const std::string context_identifiers =
        "private class ChannelAsFlow<T>(\n"
        " private val consume: Boolean,\n"
        " context: CoroutineContext = EmptyCoroutineContext,\n"
        " capacity: Int = Channel.OPTIONAL_CHANNEL\n"
        ") : ChannelFlow<T>(context, capacity) {\n"
        " fun create(context: CoroutineContext) = context\n"
        " val context = EmptyCoroutineContext\n"
        " fun identifiers(f: Int, fu: Int, function: Int, λ: Int, `fun`: Int) = f + fu + function + λ + `fun`\n"
        " fun invokeContext() = context(context)\n"
        " fun bitmask(permissionsBitmask: Int) = forbiddenElementsBitmask and permissionsBitmask == 0\n"
        "}\n";
    tree = ts_parser_parse_string(parser, nullptr, context_identifiers.data(), context_identifiers.size());
    assert(!ts_node_has_error(ts_tree_root_node(tree)));
    std::vector<TSNode> constructor_parameters;
    assert(ast_distance::kotlin_identifier_diagnostics(ts_tree_root_node(tree), context_identifiers).empty());
    collect(ts_tree_root_node(tree), "class_parameter", constructor_parameters);
    assert(constructor_parameters.size() == 3);
    assert(context_identifiers.substr(ts_node_start_byte(constructor_parameters[1]),
        ts_node_end_byte(constructor_parameters[1]) - ts_node_start_byte(constructor_parameters[1])) ==
        "context: CoroutineContext = EmptyCoroutineContext");
    ts_tree_delete(tree);
    const std::string when_branches =
        "fun check(symbol: Symbol) { when (symbol) {\n"
        " is Function -> if (!symbol.enabled) return\n"
        " else -> return\n} }\n"
        "fun choose(value: Any?): Boolean = when (value) {\n"
        " null -> false\n else if value == other -> false\n"
        " is Owner if value.enabled -> true\n else -> true\n}\n";
    tree = ts_parser_parse_string(parser, nullptr, when_branches.data(), when_branches.size());
    assert(!ts_node_has_error(ts_tree_root_node(tree)));
    std::vector<TSNode> entries, guards;
    collect(ts_tree_root_node(tree), "when_entry", entries);
    collect(ts_tree_root_node(tree), "when_entry_guard", guards);
    assert(entries.size() == 6 && guards.size() == 2);
    assert(when_branches.substr(ts_node_start_byte(entries[1]),
        ts_node_end_byte(entries[1]) - ts_node_start_byte(entries[1])).find("else -> return") == 0);
    ts_tree_delete(tree);
    const std::string nested_if =
        "fun nested(a: Boolean, b: Boolean) { if (a) if (b) yes() else no() }";
    tree = ts_parser_parse_string(parser, nullptr, nested_if.data(), nested_if.size());
    assert(!ts_node_has_error(ts_tree_root_node(tree)));
    std::vector<TSNode> nested_conditions;
    collect(ts_tree_root_node(tree), "if_expression", nested_conditions);
    assert(nested_conditions.size() == 2);
    const auto inner = nested_if.substr(ts_node_start_byte(nested_conditions[1]),
        ts_node_end_byte(nested_conditions[1]) - ts_node_start_byte(nested_conditions[1]));
    assert(inner == "if (b) yes() else no()");
    ts_tree_delete(tree);
    // Kotlin permits renaming in parentheses only; malformed delimiters remain errors.
    for (const std::string invalid : {
             "fun broken() { val [a = b, c] = pair }",
             "fun broken() { val [a, b) = pair }",
             "fun broken() { val [a,, b] = pair }",
             "fun broken() { [val a = b, var c] = pair }",
             "fun broken() { (val a, b) = pair }",
             "fun broken() { val open = 0..< }",
             "fun broken(fun: Int) {}",
             "fun broken() { a fun b }",
             "context() fun broken() {}",
             "context(owner:) fun broken() {}",
             "context(owner: Owner,,) fun broken() {}",
             "fun broken(x: Any) = when(x) { else if -> false }",
             "fun broken(x: Any) = when(x) { is Owner if -> true }"}) {
        tree = ts_parser_parse_string(parser, nullptr, invalid.data(), invalid.size());
        const bool rejected = ts_node_has_error(ts_tree_root_node(tree)) ||
            !ast_distance::kotlin_identifier_diagnostics(ts_tree_root_node(tree), invalid).empty();
        if (!rejected) {
            char* diagnostic = ts_node_string(ts_tree_root_node(tree));
            std::cerr << "Accepted invalid fixture: " << invalid << '\n' << diagnostic << '\n';
            std::free(diagnostic);
        }
        assert(rejected);
        ts_tree_delete(tree);
    }
    ts_parser_delete(parser);
}
