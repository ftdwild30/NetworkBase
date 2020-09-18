//
// Created by ftd.wild on 2020/9/17.
//

#ifndef NETWORK_BASE_F_SOCKET_PAIR_H
#define NETWORK_BASE_F_SOCKET_PAIR_H

#include <unistd.h>

namespace ftdwild30 {

/*
 * 成对的socket，本库中用于内部消息通信。
 *
 * */
class SocketPair {
public:
    SocketPair();
    ~SocketPair();

    int Start();

    //未带缓冲
    ssize_t Send(const char *buf, size_t len);

    void Stop();

private:
    friend class Engine;
    int GetReadFd() const;
    void Readable();
    bool GetCloseStatus() const;

private:
    bool close_;
    int wakeup_in_;
    int wakeup_out_;
    char *read_cache_;

private:
    static const int kRecvBufCacheLen = 1024;
};



} // namespace ftdwild30

#endif //NETWORK_BASE_F_SOCKET_PAIR_H
