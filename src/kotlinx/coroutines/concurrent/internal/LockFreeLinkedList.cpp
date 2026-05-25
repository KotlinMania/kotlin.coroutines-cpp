/**
 * Transliterated from: kotlinx-coroutines-core/concurrent/src/internal/LockFreeLinkedList.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Doubly-linked concurrent list node with remove support, based on Sundell & Tsigas
 * "Lock-Free and Practical Doubly Linked List-Based Deques Using Single-Word
 * Compare-and-Swap" with considerable upstream modifications. The full algorithm —
 * sentinel-anchored linearization on next-pointer updates, the Removed marker class for
 * tombstoned nodes, ListClosed marker for closed lists, the OpDescriptor multi-word CAS,
 * and the consolidated correctPrev helper — lives in the matching header file
 * (concurrent/internal/LockFreeLinkedList.hpp). This translation unit is the inventory
 * companion for the file pair; the algorithm is header-defined because every node
 * pointer access is templated on the concrete node type.
 *
 * Notes from upstream that bind the C++ port:
 * - Operations add to the right (tail) only; there is no left-add.
 * - Previous pointers are not marked for removal; backwards traversal is not
 *   linearizable.
 * - Remove-helping logic is consolidated in correctPrev.
 */

#include "kotlinx/coroutines/internal/LockFreeLinkedList.hpp"
