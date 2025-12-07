// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/benchmarks/jvm/kotlin/kotlinx/coroutines/BenchmarkUtils.kt
// TODO: Resolve imports and dependencies
// TODO: Implement ThreadLocalRandom or equivalent

namespace kotlinx {
namespace coroutines {

// TODO: import java.util.concurrent.*

void do_geom_distr_work(int work) {
    // We use geometric distribution here. We also checked on macbook pro 13" (2017) that the resulting work times
    // are distributed geometrically, see https://github.com/Kotlin/kotlinx.coroutines/pull/1464#discussion_r355705325
    double p = 1.0 / work;
    // TODO: ThreadLocalRandom implementation
    auto r = ThreadLocalRandom::current();
    while (true) {
        if (r.next_double() < p) break;
    }
}

} // namespace coroutines
} // namespace kotlinx
