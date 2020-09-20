//
// Created by ftd.wild on 2020/9/16.
//

#include "f_dns_service.h"

#include <assert.h>

#include "f_dns_protocol.h"
#include "f_transmission.h"
#include "f_time.h"


namespace ftdwild30 {

DnsProtocolProcess::DnsProtocolProcess(std::shared_ptr<DnsProtocolResult> result) : SocketHandler()  {
    init_ = false;
    port_ = 0;
    timeout_check_ = false;
    on_result_ = false;
    timeout_ms_ = 0;
    result_ = result;
}

DnsProtocolProcess::~DnsProtocolProcess() {

}

void DnsProtocolProcess::SetTimeout(size_t timeout_ms) {
    if (timeout_ms) {
        timeout_check_ = true;
        timeout_ms_ = Time::GetSystemTimeMs() + timeout_ms;
    } else {
        timeout_check_ = false;
    }
}

void DnsProtocolProcess::OnConnect() {
    if (!init_) {
        return;
    }

    char buf[kDnsRequestLen] = {0};
    uint32_t index = 0;
    DnsProtocol::RequestConstruct(addr_.c_str(), buf, &index);
    if (index > 0 && index < kDnsRequestLen) {
        socket_.lock()->Send(buf, index);
    } else {
        socket_.lock()->Close();
        on_result_ = true;
        result_->OnResult(-1, "", 0, 0);
    }
}

void DnsProtocolProcess::OnDisconnect() {
    if (!on_result_) {
        result_->OnResult(-1, "", 0, 0);
    }
}

ssize_t DnsProtocolProcess::OnData(const char *data, size_t len) {
    if (!init_) {
        return -1;
    }

    std::string ip;
    if (DnsProtocol::ResponseParse(data, len, ip) == 0) {
        on_result_ = true;
        size_t timeout_ms;
        if (!timeout_check_) {
            timeout_ms = 0;
        } else {
            size_t now = Time::GetSystemTimeMs();
            if ((now + kDnsTimeReserved) < timeout_ms_) {
                timeout_ms = timeout_ms_ - now;
            } else {
                timeout_ms = kDnsTimeReserved;
            }
        }
        result_->OnResult(0, ip, port_, timeout_ms);
    } else {
        on_result_ = true;
        result_->OnResult(-1, "", 0, 0);
    }
    socket_.lock()->Close();
    return len;
}

void DnsProtocolProcess::OnWrite() {

}

DnsService::DnsService() {
    init_ = false;
}

DnsService::~DnsService() {

}

void DnsService::Start() {
    resolvConfParse();
    checkDnsNameService();
}

void DnsService::GetAddrInfo(const std::string &addr,
                             uint16_t port,
                             size_t timeout_ms,
                             Engine *engine,
                             std::shared_ptr<DnsProtocolResult> result) {
    if (!init_) {
        return;
    }

    getAddrInfo(name_service_[0], addr, port, timeout_ms, engine, result);
}

void DnsService::resolvConfParse() {
    /* 读取配置 */
    FILE *fp = fopen("/etc/resolv.conf", "r");
    if (!fp) {
        return;
    }

    fseek(fp, 0, SEEK_END);
    uint32_t file_size = ftell(fp);
    if (file_size == 0) {
        fclose(fp);
        return;
    }

    char *buf = new char[file_size + 1];
    memset(buf, 0, file_size + 1);

    fseek(fp, 0, SEEK_SET);
    uint32_t read_len = fread(buf, 1, file_size, fp);
    if (read_len != file_size) {
        delete [] buf;
        fclose(fp);
        return;
    }
    fclose(fp);

    char *start = buf;
    for (;;) {
        char *new_line = strchr(start, '\n');
        if (!new_line) {
            resolvConfParseLine(start);
            break;
        } else {
            *new_line = 0;
            resolvConfParseLine(start);
            start = new_line + 1;
        }
    }
    delete [] buf;
}

void DnsService::resolvConfParseLine(char *start) {
    char *state;
    static const char *const delims = " \t";

    char *first_token = strtok_r(start, delims, &state);
    if (!first_token) {
        return;
    }

    if (!strcmp(first_token, "nameserver")) {
        const char *const name_service = strtok_r(nullptr, delims, &state);
        if (name_service) {
            name_service_.emplace_back(name_service);
        }
        /* domain/search/options是可选性，此处未处理 */
    } else if (!strcmp(first_token, "domain")) {

    } else if (!strcmp(first_token, "search")) {

    } else if (!strcmp(first_token, "options")) {

    }
}

void DnsService::checkDnsNameService() {
    if (name_service_.empty()) {//读取不到DNS服务器时，使用默认值
        name_service_.push_back("223.5.5.5");
    }
}

void DnsService::getAddrInfo(const std::string &name_service,
                             const std::string &addr,
                             uint16_t port,
                             size_t timeout_ms,
                             Engine *engine,
                             std::shared_ptr<DnsProtocolResult> result) {
    assert(engine);

    std::shared_ptr<DnsProtocolProcess> process = std::make_shared<DnsProtocolProcess>(result);
    std::shared_ptr<Transmission> trans = std::make_shared<Transmission>(process);
    process->SetAddr(addr, port);
    process->SetSocket(trans);
    process->SetTimeout(timeout_ms);
    trans->SetProtocol(1);//UDP
    trans->SetIpPort(name_service, DnsProtocol::kDnsPort);
    trans->Connect(timeout_ms);
    engine->Add(trans);
}

} // namespace ftdwild30
