//
// Created by ftd.wild on 2020/9/5.
//

#include "f_socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "f_log.h"
#include "f_socket_assistant.h"

namespace ftdwild30 {



Socket::Socket() {
    fd_ = 0;
    port_ = 0;
    connected_ = false;
    status_ = INIT;
    protocol_ = 0;
    read_cache_ = new char[kRecvCacheLen];
}

Socket::~Socket() {
    delete [] read_cache_;
}

void Socket::SetProtocol(int protocol) {
    protocol_ = protocol;
}

void Socket::SetIpPort(const std::string &ip, uint16_t port) {
    ip_ = ip;
    port_ = port;
}

void Socket::Connect() {
    if (!SocketAssistant::IpValid(ip_)) {
        LOG_ERROR("Illegal ip");
        return;
    }

    if (status_ == INIT) {
        status_ = START_CONNECTING;
    }
}

void Socket::Close() {
    status_ = DISCONNECTED;
}

void Socket::RealConnect() {
    if (status_ != START_CONNECTING) {
        return;
    }

    if (protocol_ == 0) {
        fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else {
        fd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

    if (fd_ < 0) {
        LOG_ERROR("Create socket failed");
        status_ = DISCONNECTED;
        return;
    }

    if (SocketAssistant::MakeNonBlock(fd_) < 0) {
        LOG_ERROR("Set socket non-block");
        status_ = DISCONNECTED;
        return;
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip_.c_str());
    sin.sin_port = htons(port_);

    if (connect(fd_, (struct sockaddr *)&sin, sizeof(sin) < 0)) {
        if (!SocketAssistant::ErrorWouldBlock()) {
            LOG_ERROR("connect ip failed, ip = %s, port =%d", ip_.c_str(), port_);
            status_ = DISCONNECTED;
            return;
        }
    }

    status_ = CONNECTED;
}

void Socket::RealDisconnect() {
    if (fd_) {
        close(fd_);
    }

    OnDisconnect();
}

void Socket::RealRead() {
    if (!connected_) {
        LOG_DEBUG("Socket not connected");
        return ;
    }

    bool flag = true;
    ssize_t read_ret;
    while (flag) {
        //分段读取数据
        read_ret = read(fd_, read_cache_, kRecvCacheLen);
        if (read_ret < 0) {
            if (SocketAssistant::ErrorWouldBlock()) {
                LOG_WARN("Read wait, ret = %zu", read_ret);
            } else {
                LOG_ERROR("Read error, ret = %zu", read_ret);
                status_ = DISCONNECTED;
            }
            return;
        }

        if (read_ret == 0) {
            LOG_ERROR("Read error, connect lost");
            status_ = DISCONNECTED;
            return;
        }

        if (read_ret > kRecvCacheLen) {
            LOG_ERROR("Read error, read too much");
            status_ = DISCONNECTED;
            return;
        }

        if (read_ret == kRecvCacheLen) {//数据可能未读完，需要再读一次
            flag = true;
        }
        if (read_ret < kRecvCacheLen) {//数据读完了
            flag = false;
        }

        //对外抛出数据
        if (OnRead(read_cache_, read_ret, !flag) < 0) {
            LOG_ERROR("On read data error");
            status_ = DISCONNECTED;
            return;
        }
    }
}

void Socket::RealWrite() {
    //未连接时，先确认连接状态
    if (!connected_) {
        int ret = SocketAssistant::SocketFinishConnecting(fd_);
        if (ret < 0) {
            LOG_ERROR("Socket error, shutdown");
            status_ = DISCONNECTED;
            return;
        }

        if (ret == 0) {
            LOG_WARN("Socket connect is not finished");
            return;
        }

        connected_ = true;
        OnConnect();//通知连接已完成
    }

    //处理写数据
    if (OnWrite() < 0) {
        LOG_ERROR("Write data error");
        status_ = DISCONNECTED;
        return;
    }
}

int Socket::GetFd() const {
    return fd_;
}

Socket::Status Socket::Getstatus() const {
    return status_;
}

ssize_t Socket::send(const char *buf, size_t len) {
    if (!(buf && len)) {
        return 0;
    }

    ssize_t send_len = write(fd_, buf, len);
    if (send_len <= 0) {
        LOG_ERROR("Write error, ret = %zu", send_len);
        return -1;
    }

    return send_len;
}

} // namespace ftdwild30