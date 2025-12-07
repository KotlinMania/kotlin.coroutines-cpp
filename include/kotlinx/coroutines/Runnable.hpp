#pragma once

namespace kotlinx {
namespace coroutines {

struct Runnable {
    virtual void run() = 0;
    virtual ~Runnable() = default;
};

} // namespace coroutines
} // namespace kotlinx
