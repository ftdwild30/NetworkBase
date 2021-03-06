//
// Created by ftd.wild on 2020/9/5.
//

#ifndef NETWORK_BASE_F_SOCKET_H
#define NETWORK_BASE_F_SOCKET_H

#include <stdarg.h>
#include <unistd.h>
#include <string>

namespace ftdwild30 {



//负责一个socket的管理，一般用于网络连接
class Socket {
public:
    enum Status {
        INIT = 0,
        START_CONNECT,
        CONNECTING,
        CONNECTED,
        DISCONNECTED,
    };
public:
    Socket();
    virtual ~Socket();

    // 0-TCP, 1-UDP
    void SetProtocol(int protocol);

    //设置ip,端口，不支持域名
    void SetIpPort(const std::string &ip, uint16_t port);

    //异步连接,0为无超时，>0为超时时间
    void Connect(size_t timeout_ms);

    //异步关闭
    void Close();

    //发送
    virtual ssize_t Send(const char *buf, size_t len) = 0;

protected:
    //子类使用
    ssize_t send(const char *buf, size_t len);

private:
    //友元，网络引擎使用
    void RealConnect();
    void RealDisconnect();
    void RealRead();
    void RealWrite();
    bool RealConnectCheck();
    int GetFd() const;
    Status Getstatus() const;
    friend class Engine;
private:
    //子类需要实现的纯虚函数功能

    //当连接成功时被调用
    virtual void OnConnect() = 0;
    //当连接断开时被调用
    virtual void OnDisconnect() = 0;
    //当连接读取到数据时被调用
    virtual ssize_t OnRead(const char *data, size_t len, bool finish) = 0;
    //当连接可写时被调用
    virtual ssize_t OnWrite() = 0;

private:
    char *read_cache_;
    uint16_t port_;
    int fd_;
    int protocol_;
    bool timeout_check_;
    size_t timeout_ms_;
    Status status_;
    std::string ip_;
private:
    static const size_t kRecvCacheLen = 16384;//16k
};

} // namespace ftdwild30

#endif //NETWORK_BASE_F_SOCKET_H
