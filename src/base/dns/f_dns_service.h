//
// Created by ftd.wild on 2020/9/16.
//

#ifndef NETWORK_BASE_F_DNS_SERVICE_H
#define NETWORK_BASE_F_DNS_SERVICE_H

#include <unistd.h>
#include <string>
#include <vector>

#include "f_socket_handler.h"
#include "f_engine.h"
#include "f_socket.h"

namespace ftdwild30 {

class DnsProtocolResult {
public:
    DnsProtocolResult() {}
    virtual ~DnsProtocolResult() {}

    virtual void OnResult(int result, const std::string &ip, uint16_t port) = 0;
};

/*
 * 异步的DNS解析
 * 1. 读取host（这部分未实现）
 * 2. 读取DNS服务器信息
 * 3. 使用DNS服务器，异步解析IP
 * */
class DnsProtocolProcess : public SocketHandler {
public:
    explicit DnsProtocolProcess(std::shared_ptr<DnsProtocolResult> result);
    virtual ~DnsProtocolProcess();

    void SetSocket(std::shared_ptr<Socket> socket) {socket_ = socket;init_ = true;};

    void SetAddr(const std::string &addr, uint16_t port) {addr_ = addr; port_ = port;};

private:
    //当连接成功时调用
    virtual void OnConnect();

    //当连接断开时调用
    virtual void OnDisconnect();

    //当连接收取到数据时调用
    virtual ssize_t OnData(const char *data, size_t len);

    //当连接可写时调用
    virtual void OnWrite();

private:
    bool init_;
    uint16_t port_;
    std::string addr_;
    std::shared_ptr<DnsProtocolResult> result_;
    std::weak_ptr<Socket> socket_;

private:
    static const uint32_t kDnsRequestLen = 1024;
};

class DnsService {
public:
    DnsService();
    ~DnsService();

    void Start();

    void GetAddrInfo(const std::string &addr, uint16_t port, std::shared_ptr<Engine> engine, std::shared_ptr<DnsProtocolResult> result);

private:
    void resolvConfParse();
    void resolvConfParseLine(char *start);
    void checkDnsNameService();
    static void getAddrInfo(const std::string &name_service, const std::string &addr, uint16_t port, std::shared_ptr<Engine> engine, std::shared_ptr<DnsProtocolResult> result);

private:
    bool init_;
    std::vector<std::string> name_service_;
};

} // namespace ftdwild30


#endif //NETWORK_BASE_F_DNS_SERVICE_H
