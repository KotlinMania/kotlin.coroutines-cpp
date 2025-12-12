#pragma once
#include <memory>
#include <functional>
#include <string>

namespace kotlinx {
namespace coroutines {

/**
 * @file CoroutineContext.hpp
 * @brief Coroutine context system for kotlinx.coroutines-cpp
 *
 * This file provides the context system that matches Kotlin's
 * kotlin.coroutines.CoroutineContext interface. The context is a
 * collection of elements that provide configuration and capabilities
 * to coroutines.
 *
 * Key concepts:
 * - Element: A single context element with a unique key
 * - Key: Type-safe identifier for context elements
 * - Composition: Contexts can be combined using the + operator
 * - Fold: Functional operation to process all elements
 */

/**
 * CoroutineContext - a collection of context elements.
 *
 * This is the C++ transliteration of Kotlin's CoroutineContext interface.
 * It provides a map-like structure where elements are identified by their keys.
 * 
 * The context system is essential for:
 * - Dispatchers (which thread executes the coroutine)
 * - Job hierarchy (cancellation and structured concurrency)
 * - Exception handling
 * - Coroutine names and debugging information
 *
 * Thread Safety: Context implementations are typically immutable and thread-safe.
 * Memory Management: Uses shared_ptr for automatic reference counting.
 *
 * Transliterated from: interface CoroutineContext in kotlin.coroutines.CoroutineContext
 */
class CoroutineContext : public std::enable_shared_from_this<CoroutineContext> {
public:
    virtual ~CoroutineContext() = default;

    struct Element;

    /**
     * Key - base class for context element keys.
     * 
     * Keys provide type-safe identification of context elements.
     * Each element type has its own key class.
     */
    class Key {
    public:
        const char* name = nullptr;
        explicit Key(const char* name = nullptr) : name(name) {}
        virtual ~Key() = default;
    };

    /**
     * KeyTyped - type-safe key for specific element types.
     * 
     * @tparam E The element type this key identifies
     */
    template <typename E>
    class KeyTyped : public Key {
    public:
        explicit KeyTyped(const char* name = nullptr) : Key(name) {}
    };

    /**
     * Returns the element with the given key from this context, or null if the key is not found.
     * 
     * @param key The key to look up
     * @return The element with the given key, or nullptr if not found
     */
    virtual std::shared_ptr<Element> get(Key* /*key*/) const { return nullptr; }

    /**
     * Iterates over all elements in this context.
     * 
     * This virtual method provides a way to traverse all context elements
     * without using virtual templates (which aren't supported in C++).
     * 
     * @param callback Function called for each element in the context
     */
    virtual void for_each(std::function<void(std::shared_ptr<Element>)> callback) const = 0;

    /**
     * Returns a context containing elements from this context followed by elements from other.
     * 
     * Elements from other context take precedence when keys conflict.
     * This is the C++ equivalent of Kotlin's + operator for contexts.
     * 
     * @param other The context to combine with this one
     * @return A new context containing elements from both contexts
     */
    std::shared_ptr<CoroutineContext> operator+(std::shared_ptr<CoroutineContext> other) const;
    
    /**
     * Accumulates values starting with initial value and applying operation from left to right.
     * 
     * This is the C++ equivalent of Kotlin's fold function.
     * 
     * @tparam R The result type
     * @param initial The initial value
     * @param operation Function to apply to each element
     * @return The accumulated result
     */
    template <typename R>
    R fold(R initial, std::function<R(R, std::shared_ptr<Element>)> operation) const {
        R acc = initial;
        for_each([&](std::shared_ptr<Element> e){
            acc = operation(acc, e);
        });
        return acc;
    }

    /**
     * Returns a context containing all elements from this context except the one with the given key.
     * 
     * @param key The key of the element to remove
     * @return A new context without the specified element
     */
    virtual std::shared_ptr<CoroutineContext> minus_key(Key* key) const = 0;
};

/**
 * Element - a single element of a coroutine context.
 * 
 * Each element has a unique key that identifies its type and purpose.
 * Elements are the building blocks of coroutine contexts.
 * 
 * This is the C++ transliteration of Kotlin's CoroutineContext.Element interface.
 */
struct CoroutineContext::Element : public CoroutineContext {
    /**
     * Returns the key of this context element.
     * 
     * @return The unique key identifying this element type
     */
    virtual Key* key() const = 0;
    
    /**
     * Returns the element with the given key, or null if not found.
     * 
     * For single elements, this returns this element if the key matches,
     * otherwise returns nullptr.
     * 
     * @param k The key to look up
     * @return This element if key matches, otherwise nullptr
     */
    std::shared_ptr<Element> get(Key* k) const override {
        if (this->key() == k) {
             return std::dynamic_pointer_cast<Element>(std::const_pointer_cast<CoroutineContext>(shared_from_this()));
        }
        return nullptr;
    }

    /**
     * Iterates over elements - for single elements, calls callback with this element.
     * 
     * @param callback Function called with this element
     */
    void for_each(std::function<void(std::shared_ptr<Element>)> callback) const override {
        callback(std::dynamic_pointer_cast<Element>(std::const_pointer_cast<CoroutineContext>(shared_from_this())));
    }

    /**
     * Returns a context without the element with the given key.
     * 
     * For single elements, if the key matches, returns EmptyCoroutineContext,
     * otherwise returns this element.
     * 
     * @param k The key of the element to remove
     * @return Empty context if key matches, otherwise this element
     */
    std::shared_ptr<CoroutineContext> minus_key(Key* k) const override {
        if (this->key() == k) return nullptr; // nullptr represents EmptyCoroutineContext here
        return std::const_pointer_cast<CoroutineContext>(shared_from_this());
    }
};

/**
 * AbstractCoroutineContextElement - convenience base class for context elements.
 * 
 * This class provides a simple implementation for elements that store their key.
 * Most concrete context elements will inherit from this class.
 * 
 * Usage example:
 * ```cpp
 * class MyElement : public AbstractCoroutineContextElement {
 * public:
 *     static KeyTyped<MyElement> KEY;
 *     MyElement() : AbstractCoroutineContextElement(&KEY) {}
 * };
 * ```
 */
class AbstractCoroutineContextElement : public virtual CoroutineContext::Element {
public:
    Key* key_;
    
    /**
     * Constructor with key.
     * 
     * @param key The key that identifies this element type
     */
    explicit AbstractCoroutineContextElement(Key* key) : key_(key) {}

    /**
     * Returns the key of this element.
     * 
     * @return The key provided during construction
     */
    Key* key() const override { return key_; }
};

/**
 * Extension function to extract the coroutine name from a context.
 *
 * Transliterated from: internal val CoroutineContext.coroutineName: String?
 *
 * @param context The coroutine context to query
 * @return The coroutine name if present, empty string otherwise
 */
std::string coroutine_name(const std::shared_ptr<CoroutineContext>& context);

} // namespace coroutines
} // namespace kotlinx
