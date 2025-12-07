#include "kotlinx/coroutines/core_fwd.hpp"
namespace kotlinx {namespace coroutines {namespace flow {namespace {
// import kotlinx.coroutines.internal.*// import kotlin.jvm.*
/**
 * This value is used a a surrogate `nullptr` value when needed.
 * It should never leak to the outside world.
 * Its usage typically are paired with [Symbol.unbox] usages.
 */
// @JvmFieldauto NULL = Symbol("NULL")

/**
 * Symbol to indicate that the value is not yet initialized.
 * It should never leak to the outside world.
 */
// @JvmFieldauto UNINITIALIZED = Symbol("UNINITIALIZED")

/*
 * Symbol used to indicate that the flow is complete.
 * It should never leak to the outside world.
 */
// @JvmFieldauto DONE = Symbol("DONE")

}}}} // namespace kotlinx::coroutines::flow::internal