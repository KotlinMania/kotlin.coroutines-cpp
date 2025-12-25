#pragma once

#include "tree.hpp"
#include "tensor.hpp"
#include <unordered_map>

namespace ast_distance {

/**
 * Binary Tree-LSTM implementation.
 * Ported from Stanford TreeLSTM (Lua/Torch) to C++.
 *
 * Based on: "Improved Semantic Representations From Tree-Structured
 *           Long Short-Term Memory Networks" (Tai et al., 2015)
 */
class BinaryTreeLSTM {
public:
    // Dimensions
    size_t in_dim;   // Input embedding dimension
    size_t mem_dim;  // Memory/hidden state dimension

    // Whether to use output gate
    bool gate_output = true;

    // Leaf module weights: input -> (cell, hidden)
    Tensor W_leaf_c;  // in_dim x mem_dim
    Tensor W_leaf_o;  // in_dim x mem_dim (if gate_output)

    // Composer weights for gates
    // Input gate: i = sigmoid(U_il * h_l + U_ir * h_r)
    Tensor U_i_l, U_i_r;  // mem_dim x mem_dim

    // Left forget gate: f_l = sigmoid(U_fl_l * h_l + U_fl_r * h_r)
    Tensor U_fl_l, U_fl_r;  // mem_dim x mem_dim

    // Right forget gate: f_r = sigmoid(U_fr_l * h_l + U_fr_r * h_r)
    Tensor U_fr_l, U_fr_r;  // mem_dim x mem_dim

    // Update: u = tanh(U_u_l * h_l + U_u_r * h_r)
    Tensor U_u_l, U_u_r;  // mem_dim x mem_dim

    // Output gate (if gate_output): o = sigmoid(U_o_l * h_l + U_o_r * h_r)
    Tensor U_o_l, U_o_r;  // mem_dim x mem_dim

    // State storage during forward pass
    struct NodeState {
        Tensor c;  // Cell state
        Tensor h;  // Hidden state
    };
    std::unordered_map<Tree*, NodeState> states;

    BinaryTreeLSTM(size_t input_dim, size_t memory_dim, bool use_output_gate = true)
        : in_dim(input_dim), mem_dim(memory_dim), gate_output(use_output_gate) {
        initialize_weights();
    }

    void initialize_weights() {
        float scale = std::sqrt(2.0f / static_cast<float>(mem_dim));

        // Leaf module
        W_leaf_c = Tensor::randn(in_dim, mem_dim, scale);
        if (gate_output) {
            W_leaf_o = Tensor::randn(in_dim, mem_dim, scale);
        }

        // Composer - input gate
        U_i_l = Tensor::randn(mem_dim, mem_dim, scale);
        U_i_r = Tensor::randn(mem_dim, mem_dim, scale);

        // Composer - left forget gate
        U_fl_l = Tensor::randn(mem_dim, mem_dim, scale);
        U_fl_r = Tensor::randn(mem_dim, mem_dim, scale);

        // Composer - right forget gate
        U_fr_l = Tensor::randn(mem_dim, mem_dim, scale);
        U_fr_r = Tensor::randn(mem_dim, mem_dim, scale);

        // Composer - update
        U_u_l = Tensor::randn(mem_dim, mem_dim, scale);
        U_u_r = Tensor::randn(mem_dim, mem_dim, scale);

        // Composer - output gate
        if (gate_output) {
            U_o_l = Tensor::randn(mem_dim, mem_dim, scale);
            U_o_r = Tensor::randn(mem_dim, mem_dim, scale);
        }
    }

    // Forward pass: compute hidden state for entire tree
    // inputs: vector of embeddings for leaf nodes (indexed by leaf_idx)
    Tensor forward(Tree* tree, const std::vector<Tensor>& inputs) {
        states.clear();
        forward_recursive(tree, inputs);
        return states[tree].h;
    }

private:
    void forward_recursive(Tree* node, const std::vector<Tensor>& inputs) {
        if (node->is_leaf()) {
            // Leaf module: c = W_c * x, h = o * tanh(c) or h = tanh(c)
            if (node->leaf_idx < 0 ||
                static_cast<size_t>(node->leaf_idx) >= inputs.size()) {
                // No input for this leaf, use zeros
                states[node] = {Tensor::zeros(mem_dim), Tensor::zeros(mem_dim)};
                return;
            }

            const Tensor& x = inputs[node->leaf_idx];
            Tensor c = W_leaf_c.matmul(x);

            Tensor h;
            if (gate_output) {
                Tensor o = W_leaf_o.matmul(x).sigmoid();
                h = o.hadamard(c.tanh());
            } else {
                h = c.tanh();
            }

            states[node] = {c, h};
        } else {
            // Process children first (post-order)
            for (auto& child : node->children) {
                if (child) {
                    forward_recursive(child.get(), inputs);
                }
            }

            // Get child states (default to zeros if missing)
            Tensor lc = Tensor::zeros(mem_dim);
            Tensor lh = Tensor::zeros(mem_dim);
            Tensor rc = Tensor::zeros(mem_dim);
            Tensor rh = Tensor::zeros(mem_dim);

            if (node->children.size() >= 1 && node->children[0]) {
                lc = states[node->children[0].get()].c;
                lh = states[node->children[0].get()].h;
            }
            if (node->children.size() >= 2 && node->children[1]) {
                rc = states[node->children[1].get()].c;
                rh = states[node->children[1].get()].h;
            }

            // Compute gates
            // i = sigmoid(U_i_l * h_l + U_i_r * h_r)
            Tensor i = (U_i_l.matmul(lh) + U_i_r.matmul(rh)).sigmoid();

            // f_l = sigmoid(U_fl_l * h_l + U_fl_r * h_r)
            Tensor fl = (U_fl_l.matmul(lh) + U_fl_r.matmul(rh)).sigmoid();

            // f_r = sigmoid(U_fr_l * h_l + U_fr_r * h_r)
            Tensor fr = (U_fr_l.matmul(lh) + U_fr_r.matmul(rh)).sigmoid();

            // u = tanh(U_u_l * h_l + U_u_r * h_r)
            Tensor u = (U_u_l.matmul(lh) + U_u_r.matmul(rh)).tanh();

            // c = i * u + f_l * c_l + f_r * c_r
            Tensor c = i.hadamard(u) + fl.hadamard(lc) + fr.hadamard(rc);

            Tensor h;
            if (gate_output) {
                // o = sigmoid(U_o_l * h_l + U_o_r * h_r)
                Tensor o = (U_o_l.matmul(lh) + U_o_r.matmul(rh)).sigmoid();
                h = o.hadamard(c.tanh());
            } else {
                h = c.tanh();
            }

            states[node] = {c, h};
        }
    }
};

/**
 * Siamese Tree-LSTM for computing similarity between two trees.
 * Based on the approach in ASTERIA paper.
 */
class TreeLSTMSimilarity {
public:
    BinaryTreeLSTM encoder;

    // Similarity module weights
    Tensor W_sim;  // (2 * mem_dim) x hidden_dim
    Tensor W_out;  // hidden_dim x 2 (binary classification: similar/dissimilar)

    size_t sim_hidden_dim;

    TreeLSTMSimilarity(size_t input_dim, size_t memory_dim,
                       size_t hidden_dim = 50)
        : encoder(input_dim, memory_dim, false),
          sim_hidden_dim(hidden_dim) {
        initialize_sim_weights();
    }

    void initialize_sim_weights() {
        float scale = std::sqrt(2.0f / static_cast<float>(encoder.mem_dim));
        W_sim = Tensor::randn(2 * encoder.mem_dim, sim_hidden_dim, scale);
        W_out = Tensor::randn(sim_hidden_dim, 2, scale);
    }

    // Compute similarity score between two trees
    // Returns value in [0, 1] where 1 = most similar
    float similarity(Tree* tree1, const std::vector<Tensor>& inputs1,
                     Tree* tree2, const std::vector<Tensor>& inputs2) {
        // Encode both trees
        Tensor h1 = encoder.forward(tree1, inputs1);
        Tensor h2 = encoder.forward(tree2, inputs2);

        // Compute features: |h1 - h2| and h1 * h2
        Tensor diff = (h1 - h2).abs();
        Tensor prod = h1.hadamard(h2);
        Tensor features = diff.concat(prod);

        // Pass through similarity network
        Tensor hidden = W_sim.matmul(features).sigmoid();
        Tensor output = W_out.matmul(hidden).softmax();

        // Return similarity score (second class = similar)
        return output[1];
    }

    // Simple cosine similarity (no learned weights, useful for comparison)
    float cosine_similarity(Tree* tree1, const std::vector<Tensor>& inputs1,
                            Tree* tree2, const std::vector<Tensor>& inputs2) {
        Tensor h1 = encoder.forward(tree1, inputs1);
        Tensor h2 = encoder.forward(tree2, inputs2);
        return h1.cosine_similarity(h2);
    }
};

} // namespace ast_distance
