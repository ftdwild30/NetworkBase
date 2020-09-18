//
// Created by ftd.wild on 2020/9/17.
//

#include "f_socket_pair.h"

#include <sys/socket.h>
#include <unistd.h>

#include "f_socket_assistant.h"
#include "f_log.h"

namespace ftdwild30 {


ftdwild30::SocketPair::SocketPair() {
    close_ = true;
    wakeup_in_ = 0;
    wakeup_out_ = 0;
    read_cache_ = new char [kRecvBufCacheLen];
}

ftdwild30::SocketPair::~SocketPair() {
    Stop();
    delete [] read_cache_;
}

int SocketPair::Start() {
    if (!close_) {
        return 0;
    }

    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) > 0) {
        return -1;
    }

    if (SocketAssistant::MakeNonBlock(sockets[0]) < 0 ||
        SocketAssistant::MakeNonBlock(sockets[1] < 0)) {
        close(sockets[0]);
        close(sockets[1]);
        return -1;
    }

    wakeup_in_ = sockets[0];
    wakeup_out_ = sockets[1];

    close_ = false;
    return 0;
}

ssize_t SocketPair::Send(const char *buf, size_t len) {
    if (close_) {
        return -1;
    }


    ssize_t send_len = write(wakeup_in_, buf, len);
    if (send_len < 0) {
        return -1;
    }
    if (send_len == 0) {
        return -1;
    }

    return send_len;
}

void SocketPair::Stop() {
    if (close_) {
        return;
    }

    if (wakeup_out_ > 0) {
        close(wakeup_out_);
        wakeup_out_ = 0;
    }
    if (wakeup_in_ > 0) {
        close(wakeup_in_);
        wakeup_in_ = 0;
    }

    close_ = true;
}

int SocketPair::GetReadFd() const {
    return wakeup_out_;
}

void SocketPair::Readable() {
    bool flag = true;
    ssize_t read_ret;
    while (flag) {
        //分段读取数据
        read_ret = read(wakeup_in_, read_cache_, kRecvBufCacheLen);
        if (read_ret < 0) {
            if (SocketAssistant::ErrorWouldBlock()) {
                LOG_WARN("Read wait, ret = %zu", read_ret);
            } else {
                LOG_ERROR("Read error, ret = %zu", read_ret);
                Stop();
            }
            return;
        }

        if (read_ret == 0) {
            LOG_ERROR("Read error, connect lost");
            Stop();
            return;
        }

        if (read_ret > kRecvBufCacheLen) {
            LOG_ERROR("Read error, read too much");
            Stop();
            return;
        }

        if (read_ret == kRecvBufCacheLen) {//数据可能未读完，需要再读一次
            flag = true;
        }
        if (read_ret < kRecvBufCacheLen) {//数据读完了
            flag = false;
        }
        //这里读取的数据直接丢弃掉
    }
}

bool SocketPair::GetCloseStatus() const {
    return close_;
}

} // namespace ftdwild30