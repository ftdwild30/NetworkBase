//
// Created by ftd.wild on 2020/9/17.
//

#ifndef NETWORK_BASE_F_SOCKET_ASSISTANT_H
#define NETWORK_BASE_F_SOCKET_ASSISTANT_H

#include <string>

namespace ftdwild30 {

//socket通用的API
class SocketAssistant {
public:
    SocketAssistant() = default;
    ~SocketAssistant() = default;

    static int MakeNonBlock(int fd);

    static int ErrorWouldBlock();

    static int SocketFinishConnecting(int fd);

    static bool IpValid(const std::string &ip);
};

} // namespace ftdwild30

#endif //NETWORK_BASE_F_SOCKET_ASSISTANT_H
