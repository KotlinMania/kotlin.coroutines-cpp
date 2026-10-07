/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license that can be found in the license/LICENSE.txt file.
 */
// port-lint: source compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:19-87
#pragma once
#include <cstdint>
#include "visitors/IrVisitorDispatch.hpp"
namespace org::jetbrains::kotlin::ir {
/**
 * The root interface of the IR tree. Each IR node implements this interface.
 *
 * Generated from: [org.jetbrains.kotlin.ir.generator.IrTree.rootElement]
 */
// NOTE(port): IR objects belong to the compiler and have stable identity.
// This is the compiler tree contract, not Native runtime object/frame layout.
// Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:19-87
class IrElement {
 public:
  virtual ~IrElement() = default;
/**
     * The start offset of the syntax node from which this IR node was generated,
     * in number of characters from the start of the source file. If there is no source information for this IR node,
     * the [UNDEFINED_OFFSET] constant is used. In order to get the line number and the column number from this offset,
     * [IrFileEntry.getLineNumber] and [IrFileEntry.getColumnNumber] can be used.
     *
     * @see IrFileEntry.getSourceRangeInfo
     */
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:28-28
  virtual std::int32_t start_offset() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:28-28
  virtual void set_start_offset(std::int32_t value) = 0;
/**
     * The end offset of the syntax node from which this IR node was generated,
     * in number of characters from the start of the source file. If there is no source information for this IR node,
     * the [UNDEFINED_OFFSET] constant is used. In order to get the line number and the column number from this offset,
     * [IrFileEntry.getLineNumber] and [IrFileEntry.getColumnNumber] can be used.
     *
     * @see IrFileEntry.getSourceRangeInfo
     */
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:38-38
  virtual std::int32_t end_offset() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:38-38
  virtual void set_end_offset(std::int32_t value) = 0;
/**
     * Original element before copying. Always satisfies the following
     * invariant: `this.attributeOwnerId == this.attributeOwnerId.attributeOwnerId`.
     */
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:44-44
  virtual IrElement& attribute_owner_id() const = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:44-44
  virtual void set_attribute_owner_id(IrElement& value) = 0;
/**
     * Runs the provided [visitor] on the IR subtree with the root at this node.
     *
     * @param visitor The visitor to accept.
     * @param data An arbitrary context to pass to each invocation of [visitor]'s methods.
     * @return The value returned by the topmost `visit*` invocation.
     */
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
  template <typename R, typename D>
  R accept(visitors::IrVisitor<R, D>& visitor, D data) {
    visitors::detail::TypedIrVisitorDispatch<R, D> dispatch(visitor, data);
    accept_dispatch(dispatch);
    return dispatch.take_result();
  }
/**
     * Runs the provided [transformer] on the IR subtree with the root at this node.
     *
     * @param transformer The transformer to use.
     * @param data An arbitrary context to pass to each invocation of [transformer]'s methods.
     * @return The transformed node.
     */
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62
  template <typename D>
  IrElement& transform(visitors::IrTransformer<D>& transformer, D data) {
    visitors::detail::TypedIrTransformerDispatch<D> dispatch(transformer, data);
    transform_dispatch(dispatch);
    return dispatch.take_element_result();
  }
/**
     * Runs the provided [visitor] on subtrees with roots in this node's children.
     *
     * Basically, calls `accept(visitor, data)` on each child of this node.
     *
     * Does **not** run [visitor] on this node itself.
     *
     * @param visitor The visitor for children to accept.
     * @param data An arbitrary context to pass to each invocation of [visitor]'s methods.
     */
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:74-74
  template <typename D>
  void accept_children(visitors::IrVisitor<void, D>& visitor, D data) {
    visitors::detail::TypedIrVisitorDispatch<void, D> dispatch(visitor, data);
    accept_children_dispatch(dispatch);
  }
/**
     * Recursively transforms this node's children *in place* using [transformer].
     *
     * Basically, executes `this.child = this.child.transform(transformer, data)` for each child of this node.
     *
     * Does **not** run [transformer] on this node itself.
     *
     * @param transformer The transformer to use for transforming the children.
     * @param data An arbitrary context to pass to each invocation of [transformer]'s methods.
     */
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:86-86
  template <typename D>
  void transform_children(visitors::IrTransformer<D>& transformer, D data) {
    visitors::detail::TypedIrTransformerDispatch<D> dispatch(transformer, data);
    transform_children_dispatch(dispatch);
  }
 protected:
  // NOTE(port): The source abstract generic operations remain abstract at the
  // C++ virtual boundary. Only source concrete nodes implement their dispatch.
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:53-53
  virtual void accept_dispatch(visitors::detail::IrVisitorDispatch& dispatch) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:62-62
  virtual void transform_dispatch(visitors::detail::IrTransformerDispatch& dispatch) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:74-74
  virtual void accept_children_dispatch(visitors::detail::IrVisitorDispatch& dispatch) = 0;
  // Transliterated from: compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt:86-86
  virtual void transform_children_dispatch(visitors::detail::IrTransformerDispatch& dispatch) = 0;
  friend class visitors::detail::IrVisitorDispatch;
  friend class visitors::detail::IrTransformerDispatch;
};
}  // namespace org::jetbrains::kotlin::ir
