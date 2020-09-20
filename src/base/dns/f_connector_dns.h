//
// Created by ftd.wild on 2020/9/16.
//

#ifndef NETWORK_BASE_F_CONNECTOR_DNS_H
#define NETWORK_BASE_F_CONNECTOR_DNS_H

#include "f_dns_service.h"

namespace ftdwild30 {

class ConnectorDns : public DnsProtocolResult {
public:
    explicit ConnectorDns(size_t connector);
    virtual ~ConnectorDns();

private:
    virtual void OnResult(int result, const std::string &ip, uint16_t port, size_t time_remain);

private:
    size_t connector_;
};

} // namespace ftdwild30

#endif //NETWORK_BASE_F_CONNECTOR_DNS_H
