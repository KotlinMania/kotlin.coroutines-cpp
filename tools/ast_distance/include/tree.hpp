#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace ast_distance {

// Forward declaration
struct Tree;
using TreePtr = std::shared_ptr<Tree>;

/**
 * A basic tree structure for AST representation.
 * Ported from Stanford TreeLSTM (Lua) to C++.
 */
struct Tree {
    Tree* parent = nullptr;
    std::vector<TreePtr> children;

    // Node type (normalized across languages)
    int node_type = 0;

    // For leaf nodes: index into input embeddings
    int leaf_idx = -1;

    // Optional: original node label for debugging
    std::string label;

    // Cached computations
    mutable int cached_size = -1;
    mutable int cached_depth = -1;

    Tree() = default;
    explicit Tree(int type) : node_type(type) {}
    Tree(int type, const std::string& lbl) : node_type(type), label(lbl) {}

    void add_child(TreePtr child) {
        child->parent = this;
        children.push_back(std::move(child));
        cached_size = -1;  // Invalidate cache
    }

    [[nodiscard]] size_t num_children() const {
        return children.size();
    }

    [[nodiscard]] bool is_leaf() const {
        return children.empty();
    }

    [[nodiscard]] int size() const {
        if (cached_size >= 0) return cached_size;
        int s = 1;
        for (const auto& child : children) {
            s += child->size();
        }
        cached_size = s;
        return s;
    }

    [[nodiscard]] int depth() const {
        if (cached_depth >= 0) return cached_depth;
        int d = 0;
        for (const auto& child : children) {
            d = std::max(d, child->depth());
        }
        if (!children.empty()) d += 1;
        cached_depth = d;
        return d;
    }

    // Depth-first pre-order traversal
    void traverse_preorder(const std::function<void(Tree*)>& fn) {
        fn(this);
        for (auto& child : children) {
            child->traverse_preorder(fn);
        }
    }

    // Depth-first post-order traversal (needed for bottom-up Tree-LSTM)
    void traverse_postorder(const std::function<void(Tree*)>& fn) {
        for (auto& child : children) {
            child->traverse_postorder(fn);
        }
        fn(this);
    }

    // Convert to left-child right-sibling binary tree format
    [[nodiscard]] TreePtr to_binary() const;

    // Collect all leaf nodes
    [[nodiscard]] std::vector<Tree*> get_leaves() {
        std::vector<Tree*> leaves;
        traverse_preorder([&leaves](Tree* node) {
            if (node->is_leaf()) {
                leaves.push_back(node);
            }
        });
        return leaves;
    }

    // Count nodes by type
    [[nodiscard]] std::vector<int> node_type_histogram(int num_types) const {
        std::vector<int> hist(num_types, 0);
        const_cast<Tree*>(this)->traverse_preorder([&hist, num_types](Tree* node) {
            if (node->node_type >= 0 && node->node_type < num_types) {
                hist[node->node_type]++;
            }
        });
        return hist;
    }
    // Flatten nodes of specific type (replacing them with their children)
    void flatten_node_type(int type_to_flatten) {
        if (children.empty()) return;

        std::vector<TreePtr> new_children;
        new_children.reserve(children.size());

        for (auto& child : children) {
            // Recurse first (bottom-up flattening)
            child->flatten_node_type(type_to_flatten);

            if (child->node_type == type_to_flatten) {
                // Dissolve this node, append its children to current parent
                for (auto& grand_child : child->children) {
                    grand_child->parent = this;
                    new_children.push_back(std::move(grand_child));
                }
            } else {
                new_children.push_back(std::move(child));
            }
        }

        children = std::move(new_children);
        cached_size = -1;
        cached_depth = -1;
    }
};

// Convert n-ary tree to binary (left-child right-sibling)
inline TreePtr Tree::to_binary() const {
    auto binary = std::make_shared<Tree>(node_type, label);
    binary->leaf_idx = leaf_idx;

    if (children.empty()) {
        return binary;
    }

    // First child becomes left child
    binary->children.push_back(children[0]->to_binary());
    binary->children[0]->parent = binary.get();

    // Remaining siblings chain as right children
    Tree* current = binary->children[0].get();
    for (size_t i = 1; i < children.size(); ++i) {
        auto sibling = children[i]->to_binary();
        sibling->parent = current;
        current->children.push_back(sibling);
        if (current->children.size() == 1) {
            // Add placeholder for left child if needed
            current->children.insert(current->children.begin(), nullptr);
        }
        current = current->children.back().get();
    }

    return binary;
}

} // namespace ast_distance
