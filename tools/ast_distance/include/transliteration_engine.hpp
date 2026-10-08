#pragma once
#include "ast_parser.hpp"
#include <string>
#include <vector>

namespace ast_distance {
struct TransliterationSpan {
    std::string node_type;
    int source_line = 0;
    size_t source_start = 0, source_end = 0;
    size_t target_start = 0, target_end = 0;
    bool supported = false;
};
struct TransliterationOutput {
    std::string buffer;
    std::vector<TransliterationSpan> spans;
    std::vector<std::string> diagnostics;
    size_t rule_hits = 0, rule_misses = 0;
    size_t documentation_misses = 0; // Separate from executable fallback/coverage.
    float rule_coverage = 0;
};
struct TransliteratedFunctionComparison {
    std::string source_name, target_name;
    int source_line = 0, target_line = 0;
    float normalized_logic = 0;
    std::vector<std::string> emitted_tokens, target_tokens;
};
struct TransliterationDistance {
    std::vector<TransliteratedFunctionComparison> functions;
    std::vector<std::string> missing_functions, extra_functions;
    TransliterationOutput translation;
    float translated_text_cosine = 0, translated_ast_cosine = 0;
    // Ordered documentation correspondence is also included in the primary text score.
    float documentation_parity = 0;
    float symbol_parity = 0, normalized_logic = 0, fallback_penalty = 0, score = 0;
    bool translated_parse_errors = false, target_parse_errors = false;
};
TransliterationOutput transliterate(const std::string& source, Language source_language,
                                  Language target_language);
TransliterationDistance transliteration_distance(const std::string& source, Language source_language,
    const std::string& target, Language target_language);
} // namespace ast_distance
