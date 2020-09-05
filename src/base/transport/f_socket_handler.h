//
// Created by ftd.wild on 2020/9/6.
//

#ifndef NETWORK_BASE_F_SOCKET_HANDLER_H
#define NETWORK_BASE_F_SOCKET_HANDLER_H

#include <unistd.h>

namespace ftdwild30 {

class SocketHandler {
public:
    SocketHandler() = default;
    virtual ~SocketHandler() = default;

    //当连接成功时调用
    virtual void OnConnect() = 0;

    //当连接断开时调用
    virtual void OnDisconnect() = 0;

    //当连接收取到数据时调用
    virtual ssize_t OnData(const char *data, size_t len) = 0;

    //当连接可写时调用
    virtual void OnWrite() = 0;
};



} // namespace ftdwild30

#endif //NETWORK_BASE_F_SOCKET_HANDLER_H
