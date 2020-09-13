//
// Created by ftd.wild on 2020/9/13.
//

#ifndef NETWORK_BASE_F_DNS_PROTOCOL_H
#define NETWORK_BASE_F_DNS_PROTOCOL_H

#include <stdint.h>
#include <string>

namespace ftdwild30 {

class DnsProtocol {
public:
    static void RequestConstruct(const char *domain, char *buffer, uint32_t *index);
    static int ResponseParse(const char *buffer, uint32_t len, std::string &ip);

    static const int kDnsPort = 53;
private:
    static void intToStr(const char *input, char *output);
};

} // namespace ftdwild30


#endif //NETWORK_BASE_F_DNS_PROTOCOL_H
