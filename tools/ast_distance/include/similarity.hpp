#pragma once

#include "tree.hpp"
#include "tensor.hpp"
#include "node_types.hpp"
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
     * Cosine similarity based on node type histogram.
     * Fast baseline method.
     */
    static float histogram_cosine_similarity(Tree* tree1, Tree* tree2) {
        auto hist1 = tree1->node_type_histogram(NUM_NODE_TYPES);
        auto hist2 = tree2->node_type_histogram(NUM_NODE_TYPES);

        float dot = 0.0f, norm1 = 0.0f, norm2 = 0.0f;
        for (int i = 0; i < NUM_NODE_TYPES; ++i) {
            dot += static_cast<float>(hist1[i]) * static_cast<float>(hist2[i]);
            norm1 += static_cast<float>(hist1[i]) * static_cast<float>(hist1[i]);
            norm2 += static_cast<float>(hist2[i]) * static_cast<float>(hist2[i]);
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
     * Tree edit distance (Zhang-Shasha algorithm).
     * Returns a distance, not similarity. Lower = more similar.
     */
    static int tree_edit_distance(Tree* tree1, Tree* tree2) {
        // Simplified implementation using dynamic programming
        // Full Zhang-Shasha is more complex but this gives a reasonable estimate

        std::vector<Tree*> nodes1, nodes2;
        tree1->traverse_postorder([&nodes1](Tree* n) { nodes1.push_back(n); });
        tree2->traverse_postorder([&nodes2](Tree* n) { nodes2.push_back(n); });

        int n = static_cast<int>(nodes1.size());
        int m = static_cast<int>(nodes2.size());

        // Simple DP: compare sequences of node types
        std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

        for (int i = 0; i <= n; ++i) dp[i][0] = i;
        for (int j = 0; j <= m; ++j) dp[0][j] = j;

        for (int i = 1; i <= n; ++i) {
            for (int j = 1; j <= m; ++j) {
                int cost = (nodes1[i-1]->node_type == nodes2[j-1]->node_type) ? 0 : 1;
                dp[i][j] = std::min({
                    dp[i-1][j] + 1,      // Delete
                    dp[i][j-1] + 1,      // Insert
                    dp[i-1][j-1] + cost  // Replace/Match
                });
            }
        }

        return dp[n][m];
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

    static ComparisonReport compare(Tree* tree1, Tree* tree2) {
        ComparisonReport report;

        report.size1 = tree1->size();
        report.size2 = tree2->size();
        report.depth1 = tree1->depth();
        report.depth2 = tree2->depth();

        report.hist1 = tree1->node_type_histogram(NUM_NODE_TYPES);
        report.hist2 = tree2->node_type_histogram(NUM_NODE_TYPES);

        report.cosine_sim = histogram_cosine_similarity(tree1, tree2);
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
