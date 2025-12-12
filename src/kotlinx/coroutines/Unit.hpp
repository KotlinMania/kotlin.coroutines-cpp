#pragma once

namespace kotlinx {
namespace coroutines {

struct Unit {
    bool operator==(const Unit&) const { return true; }
    bool operator!=(const Unit&) const { return false; }
};

} // namespace coroutines
} // namespace kotlinx
