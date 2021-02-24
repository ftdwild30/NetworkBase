//
// Created by ftd.wild on 2020/9/5.
//

#ifndef NETWORK_BASE_F_IO_MULTIPLEXING_H
#define NETWORK_BASE_F_IO_MULTIPLEXING_H

#include <unistd.h>
#include <sys/select.h>

#ifdef IO_MULTIPLEXING_EPOLL
#include <sys/epoll.h>
#endif

#ifdef IO_MULTIPLEXING_SELECT
#include <map>
#endif


namespace ftdwild30 {


/**
 * IO多路复用，用于屏蔽seletc/epoll/kqueue等
 */
#ifdef IO_MULTIPLEXING_SELECT
class IoMultiplexing {
public:
    IoMultiplexing();
    ~IoMultiplexing();

    int AddFd(int fd, bool read, bool write);

    int DeleteFd(int fd);

    int Dispatch(bool endless, size_t time_out_ms);

    int Sleep(size_t time_ms);

    int FdCheck(int fd, bool &read_able, bool &write_able);

private:
    fd_set *read_set_;
    fd_set *write_set_;
    int fd_max_;
    std::map<int, int> map_;

private:
    static const int kRead = 0x01;
    static const int kWrite = 0x02;
};
#endif // IO_MULTIPLEXING_SELECT

#ifdef IO_MULTIPLEXING_EPOLL
class IoMultiplexing {
public:
    IoMultiplexing();
    ~IoMultiplexing();

    int AddFd(int fd, bool read, bool write);

    int DeleteFd(int fd);

    int Dispatch(bool endless, size_t time_out_ms);

    int Sleep(size_t ms);

    int FdCheck(int fd, bool &read_able, bool &write_able);

private:
    int epoll_;
    bool init_;
    unsigned int fd_num_;
    struct epoll_event *events_;
private:
    static const unsigned int kEpollMax = 256;
};

#endif // IO_MULTIPLEXING_EPOLL

} // namespace ftdwild30

#endif //NETWORK_BASE_F_IO_MULTIPLEXING_H
