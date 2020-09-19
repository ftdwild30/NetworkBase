//
// Created by ftd.wild on 2020/9/19.
//

#ifndef NETWORK_BASE_F_TIME_H
#define NETWORK_BASE_F_TIME_H

#include <cstddef>
namespace ftdwild30 {


class Time {
public:
    Time() = delete;
    ~Time() = delete;

    static size_t  GetSystemTimeMs();
};


} // namespace ftdwild30

#endif //NETWORK_BASE_F_TIME_H
