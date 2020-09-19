//
// Created by ftd.wild on 2020/9/19.
//

#include "f_time.h"

#include <chrono>

namespace ftdwild30 {


size_t Time::GetSystemTimeMs() {
    std::chrono::steady_clock::time_point zero;
    std::chrono::steady_clock::time_point cur = std::chrono::steady_clock::now();

    return static_cast<size_t >((std::chrono::duration_cast<std::chrono::milliseconds>(cur - zero)).count());
}

} // namespace ftdwild30