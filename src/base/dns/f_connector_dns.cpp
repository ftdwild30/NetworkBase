//
// Created by ftd.wild on 2020/9/16.
//

#include "f_connector_dns.h"

#include <memory>
#include <string>

#include "f_dns_service.h"
#include "f_connector.h"


namespace ftdwild30 {

ConnectorDns::ConnectorDns(size_t connector) {
    connector_ = connector;
}

ConnectorDns::~ConnectorDns() {

}

void ConnectorDns::OnResult(int result, const std::string &ip, uint16_t port, size_t time_remain) {
    if (result != 0) {
        Connector::Close(connector_);
        return;
    }

    Connector::connectIp(connector_, ip, port, time_remain);
}

} // namespace ftdwild30
