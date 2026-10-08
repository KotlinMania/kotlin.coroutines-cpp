/** Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1423-1451 */
#pragma once

#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include <exception>
#include <memory>

namespace kotlinx::coroutines {

// Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1423-1451
class JobImpl : public JobSupport, public CompletableJob {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1425-1426
    // NOTE(port): Parent attachment requires shared_from_this; create performs
    // the source initialization after shared ownership has been established.
    explicit JobImpl(std::shared_ptr<Job> parent);
    // Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1425-1438
    static std::shared_ptr<JobImpl> create(std::shared_ptr<Job> parent);
    // Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1426-1438
    void init_parent_job(std::shared_ptr<Job> parent) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1439
    bool complete() override;
    // Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1440-1441
    bool complete_exceptionally(std::exception_ptr exception) override;
    ~JobImpl() override = default;

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1427
    bool get_on_cancel_complete() const override;
    // Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1438
    bool get_handles_exception() const override;

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1443-1450
    bool handles_exception() const;
    bool handles_exception_ = false;
};

} // namespace kotlinx::coroutines
