//
// Created by ftd.wild on 2020/9/6.
//

#ifndef NETWORK_BASE_F_TRANSMISSION_H
#define NETWORK_BASE_F_TRANSMISSION_H

#include <memory>

#include "f_socket_handler.h"
#include "f_socket.h"
#include "f_auto_grow_buffer.h"

namespace ftdwild30 {

class Transmission : public Socket {
public:
    explicit Transmission(std::shared_ptr<SocketHandler> handler);
    virtual ~Transmission();

    virtual ssize_t Send(const char *buf, size_t len);

private:
    //当连接成功时被调用
    virtual void OnConnect();
    //当连接断开时被调用
    virtual void OnDisconnect();
    //当连接读取到数据时被调用
    virtual ssize_t OnRead(const char *data, size_t len, bool finish);

    //当连接可写时被调用
    virtual ssize_t OnWrite();

private:
    ssize_t writeBufferPopData(const char *buf, size_t len);
    ssize_t readBufferPopData(const char *buf, size_t len);

private:
    AutoGrowBuffer *send_;
    AutoGrowBuffer *read_;
    std::shared_ptr<SocketHandler> handler_;

private:
    static const size_t kSendBufLen = 64 * 1024;
    static const size_t kReadBufLen = 64 * 1024;
};



} // namespace ftdwild30


#endif //NETWORK_BASE_F_TRANSMISSION_H
