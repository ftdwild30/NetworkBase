//
// Created by ftd.wild on 2020/9/16.
//

#include "f_connector.h"

#include "f_engine.h"
#include "f_dns_service.h"
#include "f_transmission.h"
#include "f_transmission_security.h"
#include "f_socket_assistant.h"
#include "f_connector_dns.h"

namespace ftdwild30 {

bool Connector::init_ = false;
Engine *Connector::engine_ = nullptr;
DnsService *Connector::dns_ = nullptr;

int Connector::Start() {
    if (init_) {
        return 0;
    }

    engine_ = new Engine();
    if (engine_->Start() < 0) {
        delete engine_;
        return -1;
    }

    dns_ = new DnsService();
    dns_->Start();

    init_ = true;
    return 0;
}

void Connector::Stop() {
    if (!init_) {
        return;
    }

    engine_->Stop();
    delete engine_;
    delete dns_;
    init_ = false;
}

size_t Connector::Create(std::shared_ptr<SocketHandler> handler, int protocol, const char *ca, size_t ca_len) {
    if (!init_) {
        return -1;
    }

    std::shared_ptr<Socket> socket;
    if (ca && ca_len) {
        std::shared_ptr<TransmissionSecurity> trans = std::make_shared<TransmissionSecurity>(handler);
        trans->SetSecurityLicence(ca, ca_len);
        socket = trans;
    } else {
        std::shared_ptr<Transmission> trans = std::make_shared<Transmission>(handler);
        socket = trans;
    }

    socket->SetProtocol(protocol);
    return engine_->Add(socket);
}

void Connector::Connect(size_t connector, const std::string &addr, uint16_t port, bool force_ip) {
    if (!init_) {
        return;
    }

    std::shared_ptr<Socket> socket;
    if (SocketAssistant::IpValid(addr) || force_ip) {
        if (engine_->Query(connector, socket)) {
            socket->SetIpPort(addr, port);
            socket->Connect();
        }
    } else {
        std::shared_ptr<ConnectorDns> dns = std::make_shared<ConnectorDns>(connector);
        dns_->GetAddrInfo(addr, port, engine_, dns);
    }
}

void Connector::Close(size_t connector) {
    if (!init_) {
        return;
    }

    std::shared_ptr<Socket> socket;
    if (engine_->Query(connector, socket)) {
        socket->Close();
    }
}

ssize_t Connector::Send(size_t connector, const char *buf, size_t len) {
    if (!init_) {
        return -1;
    }

    std::shared_ptr<Socket> socket;
    if (engine_->Query(connector, socket)) {
        return socket->Send(buf, len);
    }

    return -1;
}

} // namespace ftdwild30
