#pragma once

#include "tree.hpp"
#include "tensor.hpp"
#include "node_types.hpp"
#include "ast_parser.hpp"
#include <cmath>
#include <algorithm>

namespace ast_distance {

/**
 * Compute various similarity metrics between ASTs.
 */
class ASTSimilarity {
public:
    static constexpr int NUM_NODE_TYPES = static_cast<int>(NodeType::NUM_TYPES);

    /**
     * Get semantic weight for a node type.
     * Structural nodes (CLASS, FUNCTION, IF) have higher weight.
     * Common nodes (VARIABLE, VAR_DECL) have lower weight.
     */
    static float get_node_weight(int node_type) {
        NodeType nt = static_cast<NodeType>(node_type);

        // Critical structural elements: weight 5.0
        if (nt == NodeType::CLASS || nt == NodeType::STRUCT ||
            nt == NodeType::FUNCTION || nt == NodeType::IF ||
            nt == NodeType::WHILE || nt == NodeType::FOR ||
            nt == NodeType::SWITCH || nt == NodeType::TRY ||
            nt == NodeType::INTERFACE || nt == NodeType::ENUM) {
            return 5.0f;
        }

        // Important operations: weight 2.0
        if (nt == NodeType::CALL || nt == NodeType::METHOD_CALL ||
            nt == NodeType::RETURN || nt == NodeType::THROW ||
            nt == NodeType::LAMBDA || nt == NodeType::COMPARISON_OP ||
            nt == NodeType::LOGICAL_OP) {
            return 2.0f;
        }

        // Operators: weight 1.5 (semantic awareness improvement)
        if (nt == NodeType::ARITHMETIC_OP || nt == NodeType::BITWISE_OP ||
            nt == NodeType::ASSIGNMENT_OP) {
            return 1.5f;
        }

        // Common/boilerplate: weight 0.5
        if (nt == NodeType::VARIABLE || nt == NodeType::VAR_DECL) {
            return 0.5f;
        }

        // Default weight
        return 1.0f;
    }

    /**
     * Weighted cosine similarity based on node type histogram.
     * Uses semantic importance weighting to prioritize structural differences.
     */
    static float histogram_cosine_similarity(Tree* tree1, Tree* tree2) {
        auto hist1 = tree1->node_type_histogram(NUM_NODE_TYPES);
        auto hist2 = tree2->node_type_histogram(NUM_NODE_TYPES);

        float dot = 0.0f, norm1 = 0.0f, norm2 = 0.0f;
        for (int i = 0; i < NUM_NODE_TYPES; ++i) {
            float weight = get_node_weight(i);
            float weighted1 = weight * static_cast<float>(hist1[i]);
            float weighted2 = weight * static_cast<float>(hist2[i]);

            dot += weighted1 * weighted2;
            norm1 += weighted1 * weighted1;
            norm2 += weighted2 * weighted2;
        }

        if (norm1 < 1e-8f || norm2 < 1e-8f) return 0.0f;
        return dot / (std::sqrt(norm1) * std::sqrt(norm2));
    }

    /**
     * Macro-friendly cosine similarity.
     *
     * Rust macro-heavy files (e.g. `macro_rules!`) often do not produce comparable AST shapes
     * across languages because the Rust parser represents macro bodies as token trees.
     * For such files we compute a cosine similarity over a small subset of node types
     * (VARIABLE + UNKNOWN), which better captures whether the port contains a similar set of
     * identifiers/tokens without over-penalizing language-specific structure.
     */
    static float histogram_cosine_similarity_macro(Tree* tree1, Tree* tree2) {
        auto hist1 = tree1->node_type_histogram(NUM_NODE_TYPES);
        auto hist2 = tree2->node_type_histogram(NUM_NODE_TYPES);

        float dot = 0.0f, norm1 = 0.0f, norm2 = 0.0f;
        for (int i = 0; i < NUM_NODE_TYPES; ++i) {
            NodeType nt = static_cast<NodeType>(i);
            float weight = 0.0f;
            if (nt == NodeType::VARIABLE || nt == NodeType::UNKNOWN) {
                weight = 1.0f;
            }

            float weighted1 = weight * static_cast<float>(hist1[i]);
            float weighted2 = weight * static_cast<float>(hist2[i]);

            dot += weighted1 * weighted2;
            norm1 += weighted1 * weighted1;
            norm2 += weighted2 * weighted2;
        }

        if (norm1 < 1e-8f || norm2 < 1e-8f) return 0.0f;
        return dot / (std::sqrt(norm1) * std::sqrt(norm2));
    }

    /**
     * Jaccard similarity of node type sets.
     */
    static float node_type_jaccard(Tree* tree1, Tree* tree2) {
        auto hist1 = tree1->node_type_histogram(NUM_NODE_TYPES);
        auto hist2 = tree2->node_type_histogram(NUM_NODE_TYPES);

        int intersection = 0, union_count = 0;
        for (int i = 0; i < NUM_NODE_TYPES; ++i) {
            intersection += std::min(hist1[i], hist2[i]);
            union_count += std::max(hist1[i], hist2[i]);
        }

        if (union_count == 0) return 1.0f;
        return static_cast<float>(intersection) / static_cast<float>(union_count);
    }

    /**
     * Structure similarity based on tree shape.
     * Compares depth, size, and branching factor.
     */
    static float structure_similarity(Tree* tree1, Tree* tree2) {
        int size1 = tree1->size();
        int size2 = tree2->size();
        int depth1 = tree1->depth();
        int depth2 = tree2->depth();

        // Size similarity (normalized)
        float size_sim = 1.0f - std::abs(size1 - size2) /
                         static_cast<float>(std::max(size1, size2));

        // Depth similarity
        float depth_sim = 1.0f - std::abs(depth1 - depth2) /
                          static_cast<float>(std::max(depth1, depth2));

        // Combine
        return 0.5f * size_sim + 0.5f * depth_sim;
    }

    /**
     * Combined similarity score using multiple metrics.
     * SHAPE-ONLY version (no identifier content). Used as a structural baseline.
     *
     * WARNING: This metric cannot distinguish real code from placeholder stubs.
     * A file of `fun x() = null` scores the same as `fun computeHash() = ...`.
     * Use combined_similarity_with_content() for real porting assessment.
     */
    static float combined_similarity(Tree* tree1, Tree* tree2,
                                     float hist_weight = 0.5f,
                                     float struct_weight = 0.3f,
                                     float jaccard_weight = 0.2f) {
        float hist_sim = histogram_cosine_similarity(tree1, tree2);
        float struct_sim = structure_similarity(tree1, tree2);
        float jaccard_sim = node_type_jaccard(tree1, tree2);

        return hist_weight * hist_sim +
               struct_weight * struct_sim +
               jaccard_weight * jaccard_sim;
    }

    /**
     * Content-aware combined similarity.
     * Uses identifier comparison as the DOMINANT signal at the function level.
     *
     * A real port reuses the same identifiers (with naming convention changes).
     * A file of stubs has completely different identifiers and scores near 0.
     *
     * Weights:
     *   0.50 - Canonical identifier cosine (the killer metric)
     *   0.15 - Canonical identifier jaccard (set overlap)
     *   0.15 - AST histogram cosine (structural shape)
     *   0.10 - Node type jaccard
     *   0.10 - Structure similarity (tree size/depth)
     */
    static float combined_similarity_with_content(
            Tree* tree1, Tree* tree2,
            const IdentifierStats& ids1, const IdentifierStats& ids2) {

        float id_cosine = ids1.canonical_cosine_similarity(ids2);
        float id_jaccard = ids1.canonical_jaccard_similarity(ids2);
        float hist_sim = histogram_cosine_similarity(tree1, tree2);
        float jaccard_sim = node_type_jaccard(tree1, tree2);
        float struct_sim = structure_similarity(tree1, tree2);

        float base =
            0.50f * id_cosine +
            0.15f * id_jaccard +
            0.15f * hist_sim +
            0.10f * jaccard_sim +
            0.10f * struct_sim;

        // Module-marker heuristic:
        //
        // Rust module root files are often "just mod declarations" with little to no executable
        // logic. Kotlin transliterations frequently represent these as empty `object` markers
        // (see ast_parser.hpp: object_declaration -> PACKAGE when empty).
        //
        // For these files, identifier agreement (module names) should dominate; otherwise we
        // systematically underrate good transliterations because Rust `mod_item` and Kotlin
        // declarations differ structurally.
        auto h1 = tree1->node_type_histogram(NUM_NODE_TYPES);
        auto h2 = tree2->node_type_histogram(NUM_NODE_TYPES);

        int structural1 =
            h1[static_cast<int>(NodeType::CLASS)] +
            h1[static_cast<int>(NodeType::STRUCT)] +
            h1[static_cast<int>(NodeType::FUNCTION)] +
            h1[static_cast<int>(NodeType::INTERFACE)] +
            h1[static_cast<int>(NodeType::ENUM)];
        int structural2 =
            h2[static_cast<int>(NodeType::CLASS)] +
            h2[static_cast<int>(NodeType::STRUCT)] +
            h2[static_cast<int>(NodeType::FUNCTION)] +
            h2[static_cast<int>(NodeType::INTERFACE)] +
            h2[static_cast<int>(NodeType::ENUM)];

        int pkg1 = h1[static_cast<int>(NodeType::PACKAGE)];
        int pkg2 = h2[static_cast<int>(NodeType::PACKAGE)];

        int max_size = std::max(tree1->size(), tree2->size());
        if (max_size <= 250 && structural1 == 0 && structural2 == 0 && (pkg1 + pkg2) >= 2) {
            // Blend identifiers, then remap [0.70, 1.00] -> [0.85, 1.00] to avoid over-boosting
            // weak matches while allowing strong module-name matches to clear the "complete port"
            // threshold.
            float marker = 0.70f * id_cosine + 0.30f * id_jaccard;
            float boosted = marker;
            if (marker >= 0.70f) {
                boosted = 0.85f + 0.15f * ((marker - 0.70f) / 0.30f);
            }
            return std::max(base, boosted);
        }

        return base;
    }

    /**
     * Maximum nodes for full edit distance. Beyond this, use strided sampling.
     * 2000 × 2000 two-row DP = 16KB — safe everywhere.
     */
    static constexpr int MAX_EDIT_DISTANCE_NODES = 2000;

    /**
     * Core DP for edit distance on node-type sequences.
     * Uses two-row rolling DP: O(m) memory instead of O(n*m).
     */
    static int edit_distance_dp(const std::vector<Tree*>& nodes1,
                                const std::vector<Tree*>& nodes2) {
        int n = static_cast<int>(nodes1.size());
        int m = static_cast<int>(nodes2.size());

        std::vector<int> prev(m + 1), curr(m + 1);
        for (int j = 0; j <= m; ++j) prev[j] = j;

        for (int i = 1; i <= n; ++i) {
            curr[0] = i;
            for (int j = 1; j <= m; ++j) {
                int cost = (nodes1[i-1]->node_type == nodes2[j-1]->node_type) ? 0 : 1;
                curr[j] = std::min({
                    prev[j] + 1,        // Delete
                    curr[j-1] + 1,      // Insert
                    prev[j-1] + cost    // Replace/Match
                });
            }
            std::swap(prev, curr);
        }
        return prev[m];
    }

    /**
     * Tree edit distance with OOM protection.
     *
     * For trees with more than MAX_EDIT_DISTANCE_NODES nodes, samples
     * every Kth node to fit within budget, then scales the result.
     * Two-row DP keeps memory at O(min(n,m)) regardless.
     */
    static int tree_edit_distance(Tree* tree1, Tree* tree2) {
        std::vector<Tree*> nodes1, nodes2;
        tree1->traverse_postorder([&nodes1](Tree* n) { nodes1.push_back(n); });
        tree2->traverse_postorder([&nodes2](Tree* n) { nodes2.push_back(n); });

        int full_n = static_cast<int>(nodes1.size());
        int full_m = static_cast<int>(nodes2.size());

        if (full_n <= MAX_EDIT_DISTANCE_NODES && full_m <= MAX_EDIT_DISTANCE_NODES) {
            return edit_distance_dp(nodes1, nodes2);
        }

        // Strided sampling
        int stride1 = (full_n + MAX_EDIT_DISTANCE_NODES - 1) / MAX_EDIT_DISTANCE_NODES;
        int stride2 = (full_m + MAX_EDIT_DISTANCE_NODES - 1) / MAX_EDIT_DISTANCE_NODES;
        int stride = std::max(stride1, stride2);

        std::vector<Tree*> s1, s2;
        for (int i = 0; i < full_n; i += stride) s1.push_back(nodes1[i]);
        for (int i = 0; i < full_m; i += stride) s2.push_back(nodes2[i]);

        return edit_distance_dp(s1, s2) * stride;
    }

    /**
     * Normalized tree edit distance (0 to 1, where 1 = identical).
     */
    static float normalized_edit_distance(Tree* tree1, Tree* tree2) {
        int dist = tree_edit_distance(tree1, tree2);
        int max_size = std::max(tree1->size(), tree2->size());
        if (max_size == 0) return 1.0f;
        return 1.0f - static_cast<float>(dist) / static_cast<float>(max_size);
    }

    /**
     * Generate detailed comparison report.
     */
    struct ComparisonReport {
        float cosine_sim;
        float structure_sim;
        float jaccard_sim;
        float edit_distance_sim;
        float combined_score;

        int size1, size2;
        int depth1, depth2;

        std::vector<int> hist1;
        std::vector<int> hist2;

        void print() const {
            printf("=== AST Similarity Report ===\n");
            printf("Tree 1: size=%d, depth=%d\n", size1, depth1);
            printf("Tree 2: size=%d, depth=%d\n", size2, depth2);
            printf("\nSimilarity Metrics:\n");
            printf("  Cosine (histogram):    %.4f\n", cosine_sim);
            printf("  Structure:             %.4f\n", structure_sim);
            printf("  Jaccard:               %.4f\n", jaccard_sim);
            printf("  Edit Distance (norm):  %.4f\n", edit_distance_sim);
            printf("  Combined Score:        %.4f\n", combined_score);
        }
    };

    static ComparisonReport compare(Tree* tree1, Tree* tree2, bool macro_friendly = false) {
        ComparisonReport report;

        report.size1 = tree1->size();
        report.size2 = tree2->size();
        report.depth1 = tree1->depth();
        report.depth2 = tree2->depth();

        report.hist1 = tree1->node_type_histogram(NUM_NODE_TYPES);
        report.hist2 = tree2->node_type_histogram(NUM_NODE_TYPES);

        report.cosine_sim = macro_friendly
            ? histogram_cosine_similarity_macro(tree1, tree2)
            : histogram_cosine_similarity(tree1, tree2);
        report.structure_sim = structure_similarity(tree1, tree2);
        report.jaccard_sim = node_type_jaccard(tree1, tree2);
        report.edit_distance_sim = normalized_edit_distance(tree1, tree2);

        report.combined_score = 0.3f * report.cosine_sim +
                                0.2f * report.structure_sim +
                                0.2f * report.jaccard_sim +
                                0.3f * report.edit_distance_sim;

        return report;
    }
};

} // namespace ast_distance
