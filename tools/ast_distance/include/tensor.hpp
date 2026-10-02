#pragma once

#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <random>

namespace ast_distance {

/**
 * Simple tensor class for Tree-LSTM computations.
 * Lightweight alternative to Eigen for this specific use case.
 */
class Tensor {
public:
    std::vector<float> data;
    size_t rows = 0;
    size_t cols = 0;

    Tensor() = default;

    Tensor(size_t r, size_t c) : data(r * c, 0.0f), rows(r), cols(c) {}

    explicit Tensor(size_t size) : data(size, 0.0f), rows(size), cols(1) {}

    Tensor(size_t r, size_t c, float val) : data(r * c, val), rows(r), cols(c) {}

    // Vector constructor
    explicit Tensor(const std::vector<float>& vec)
        : data(vec), rows(vec.size()), cols(1) {}

    [[nodiscard]] size_t size() const { return data.size(); }

    [[nodiscard]] bool is_vector() const { return cols == 1; }

    float& operator()(size_t i, size_t j = 0) {
        return data[i * cols + j];
    }

    [[nodiscard]] float operator()(size_t i, size_t j = 0) const {
        return data[i * cols + j];
    }

    float& operator[](size_t i) { return data[i]; }
    [[nodiscard]] float operator[](size_t i) const { return data[i]; }

    // Initialize with zeros
    static Tensor zeros(size_t size) {
        return Tensor(size);
    }

    static Tensor zeros(size_t r, size_t c) {
        return Tensor(r, c, 0.0f);
    }

    // Initialize with random values (Xavier initialization)
    static Tensor randn(size_t r, size_t c, float scale = 1.0f) {
        static std::mt19937 gen(42);
        std::normal_distribution<float> dist(0.0f, scale);

        Tensor t(r, c);
        for (auto& v : t.data) {
            v = dist(gen);
        }
        return t;
    }

    // Element-wise operations
    Tensor operator+(const Tensor& other) const {
        if (size() != other.size()) {
            throw std::runtime_error("Tensor size mismatch in addition");
        }
        Tensor result(rows, cols);
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = data[i] + other.data[i];
        }
        return result;
    }

    Tensor operator-(const Tensor& other) const {
        if (size() != other.size()) {
            throw std::runtime_error("Tensor size mismatch in subtraction");
        }
        Tensor result(rows, cols);
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = data[i] - other.data[i];
        }
        return result;
    }

    // Element-wise multiplication (Hadamard product)
    Tensor hadamard(const Tensor& other) const {
        if (size() != other.size()) {
            throw std::runtime_error("Tensor size mismatch in Hadamard product");
        }
        Tensor result(rows, cols);
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = data[i] * other.data[i];
        }
        return result;
    }

    // Scalar multiplication
    Tensor operator*(float scalar) const {
        Tensor result(rows, cols);
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = data[i] * scalar;
        }
        return result;
    }

    // Matrix-vector multiplication
    Tensor matmul(const Tensor& vec) const {
        if (cols != vec.rows) {
            throw std::runtime_error("Matrix-vector dimension mismatch");
        }
        Tensor result(rows);
        for (size_t i = 0; i < rows; ++i) {
            float sum = 0.0f;
            for (size_t j = 0; j < cols; ++j) {
                sum += (*this)(i, j) * vec[j];
            }
            result[i] = sum;
        }
        return result;
    }

    // Dot product (for vectors)
    [[nodiscard]] float dot(const Tensor& other) const {
        if (size() != other.size()) {
            throw std::runtime_error("Vector size mismatch in dot product");
        }
        float sum = 0.0f;
        for (size_t i = 0; i < size(); ++i) {
            sum += data[i] * other.data[i];
        }
        return sum;
    }

    // L2 norm
    [[nodiscard]] float norm() const {
        float sum = 0.0f;
        for (float v : data) {
            sum += v * v;
        }
        return std::sqrt(sum);
    }

    // Cosine similarity
    [[nodiscard]] float cosine_similarity(const Tensor& other) const {
        float dot_prod = dot(other);
        float norm_a = norm();
        float norm_b = other.norm();
        if (norm_a < 1e-8f || norm_b < 1e-8f) return 0.0f;
        return dot_prod / (norm_a * norm_b);
    }

    // Activation functions
    [[nodiscard]] Tensor sigmoid() const {
        Tensor result(rows, cols);
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = 1.0f / (1.0f + std::exp(-data[i]));
        }
        return result;
    }

    [[nodiscard]] Tensor tanh() const {
        Tensor result(rows, cols);
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = std::tanh(data[i]);
        }
        return result;
    }

    [[nodiscard]] Tensor relu() const {
        Tensor result(rows, cols);
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = std::max(0.0f, data[i]);
        }
        return result;
    }

    // Softmax
    [[nodiscard]] Tensor softmax() const {
        Tensor result(rows, cols);
        float max_val = *std::max_element(data.begin(), data.end());
        float sum = 0.0f;
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = std::exp(data[i] - max_val);
            sum += result.data[i];
        }
        for (auto& v : result.data) {
            v /= sum;
        }
        return result;
    }

    // Concatenate two vectors
    [[nodiscard]] Tensor concat(const Tensor& other) const {
        Tensor result(size() + other.size());
        std::copy(data.begin(), data.end(), result.data.begin());
        std::copy(other.data.begin(), other.data.end(),
                  result.data.begin() + static_cast<long>(size()));
        return result;
    }

    // Absolute value
    [[nodiscard]] Tensor abs() const {
        Tensor result(rows, cols);
        for (size_t i = 0; i < size(); ++i) {
            result.data[i] = std::abs(data[i]);
        }
        return result;
    }
};

} // namespace ast_distance
