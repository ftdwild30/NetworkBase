//
// Created by ftd.wild on 2020/9/16.
//

#ifndef NETWORK_BASE_F_CONNECTOR_H
#define NETWORK_BASE_F_CONNECTOR_H

#include <memory>
#include <string>

#include "f_socket_handler.h"


namespace ftdwild30 {

/*
 * 这个类用于避免socket和SocketHandler在编程中已引起循环引用的情况，降低编程复杂度。
 * 可以用于屏蔽socket走TCP/UDP/TLS的差异
 * 可以屏蔽域名和IP的差异
 * */
class Engine;
class DnsService;
class Connector {
public:
    Connector() = delete;
    ~Connector() = delete;

    static int Start();

    static void Stop();

    static size_t Create(std::shared_ptr<SocketHandler> handler, int protocol, const char *ca, size_t ca_len);

    static void Connect(size_t connector, const std::string &addr, uint16_t port, size_t timeout_ms = 0);

    static void Close(size_t connector);

    static ssize_t Send(size_t connector, const char *buf, size_t len);

private:
    static void connectIp(size_t connector, const std::string &addr, uint16_t port, size_t timeout_ms);
    friend class ConnectorDns;

private:
    static bool init_;
    static Engine *engine_;
    static DnsService *dns_;
};



} // namespace ftdwild30

#endif //NETWORK_BASE_F_CONNECTOR_H
