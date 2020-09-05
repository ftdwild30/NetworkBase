//
// Created by ftd.wild on 2020/9/5.
//

#include "f_socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "f_log.h"

namespace ftdwild30 {

int SocketAssistant::MakeNonBlock(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL, nullptr)) < 0) {
        LOG_ERROR("make non block failed");
        return -1;
    }

    if (!(flags & O_NONBLOCK)) {
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            LOG_ERROR("make non block failed");
            return -1;
        }
    }

    return 0;
}

int SocketAssistant::ErrorWouldBlock() {
    //errno是线程私有数据（TSD)，因此不需要传参
    return (errno == EAGAIN || errno == EINTR || errno == EINPROGRESS);
}

int SocketAssistant::SocketFinishConnecting(int fd) {
    int e;
    socklen_t e_len = sizeof(e);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&e, &e_len) < 0) {
        return -1;
    }

    if (e) {
        if ((e == EINTR) || (e == EINPROGRESS)) {
            return 0;
        }
        errno = e;
        return -1;
    }

    return 1;
}

bool SocketAssistant::IpValid(const std::string &ip) {
    int part_1, part_2, part_3, part_4, num;

    num = sscanf(ip.c_str(), "%d.%d.%d.%d", &part_1, &part_2, &part_3, &part_4);
    if (num != 4) {
        return false;
    }

    if (part_1 > 255 || part_2 > 255 || part_3 > 255 || part_4 > 255) {
        return false;
    }

    for (size_t i = 0; i < ip.length(); i++) {
        if ((ip[i] != '.') && ((ip[i] < '0') || (ip[i] > '9'))) {
            return false;
        }
    }

    return true;
}

Socket::Socket() {

}

Socket::~Socket() {

}

void Socket::SetProtocol(int protocol) {

}

void Socket::SetIpPort(const std::string &ip, uint16_t port) {

}

void Socket::Connect() {

}

void Socket::Close() {

}

void Socket::RealConnect() {

}

void Socket::RealDisconnect() {

}

void Socket::RealRead() {

}

void Socket::RealWrite() {

}

int Socket::GetFd() const {
    return 0;
}

Socket::Status Socket::Getstatus() const {
    return START_CONNECTING;
}

ssize_t Socket::send(const char *buf, size_t len) {
    return 0;
}

} // namespace ftdwild30