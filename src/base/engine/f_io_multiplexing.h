//
// Created by ftd.wild on 2020/9/5.
//

#ifndef NETWORK_BASE_F_IO_MULTIPLEXING_H
#define NETWORK_BASE_F_IO_MULTIPLEXING_H

#include <unistd.h>

namespace ftdwild30 {


/**
 * IO多路复用，用于屏蔽seletc/epoll/kqueue等
 */
class IoMultiplexing {
public:
    IoMultiplexing();
    ~IoMultiplexing();

    void AddFd(int fd, bool read, bool write);

    int Dispatch(bool endless, size_t time_out_ms);

    void Clear();

    bool IsReadable(int fd);

    bool IsWritable(int fd);

private:
    fd_set *read_set_;
    fd_set *write_set_;
    int fd_max_;
};


} // namespace ftdwild30

#endif //NETWORK_BASE_F_IO_MULTIPLEXING_H
