//
// Created by ftd.wild on 2020/9/13.
//

#include "f_dns_protocol.h"

#include <string.h>

namespace ftdwild30 {

#define DNS_ID_FIELD                (0x6666)
#define DNS_CTRL_FIELD              (0x0100)
#define DNS_QUESTION_COUNT_FILED    (0x0001)
#define DNS_ANSWER_COUNT_FIELD      (0x0000)
#define DNS_AUTHORITY_COUNT_FIELD   (0x0000)
#define DNS_ADDITIONAL_COUNT_FIELD  (0x0000)
#define DNS_QTYPE_FIELD             (0x0001)
#define DNS_QCLASS_FIELD            (0x0001)

void DnsProtocol::RequestConstruct(const char *domain, char *buffer, uint32_t *index) {
    uint32_t i = 0;

    buffer[i++] = (DNS_ID_FIELD >> 8) & 0x00FF;
    buffer[i++] = DNS_ID_FIELD & 0x00FF;

    buffer[i++] = (DNS_CTRL_FIELD >> 8) & 0x00FF;
    buffer[i++] = DNS_CTRL_FIELD & 0x00FF;

    buffer[i++] = (DNS_QUESTION_COUNT_FILED >> 8) & 0x00FF;
    buffer[i++] = DNS_QUESTION_COUNT_FILED & 0x00FF;

    buffer[i++] = (DNS_ANSWER_COUNT_FIELD >> 8) & 0x00FF;
    buffer[i++] = DNS_ANSWER_COUNT_FIELD & 0x00FF;

    buffer[i++] = (DNS_AUTHORITY_COUNT_FIELD >> 8) & 0x00FF;
    buffer[i++] = DNS_AUTHORITY_COUNT_FIELD & 0x00FF;

    buffer[i++] = (DNS_ADDITIONAL_COUNT_FIELD >> 8) & 0x00FF;
    buffer[i++] = DNS_ADDITIONAL_COUNT_FIELD & 0x00FF;

    uint32_t section_start = 0;
    uint32_t qname_index = 0;
    do {
        if (domain[qname_index] == '.' || qname_index == strlen(domain)) {
            buffer[i++] = (uint32_t) (qname_index - section_start);
            memcpy(&buffer[i], &domain[section_start], (qname_index - section_start));
            i += (qname_index - section_start);
            section_start = qname_index + 1;
        }

        if (qname_index == strlen(domain)) {
            break;
        }

        qname_index++;
    } while (1);
    buffer[i++] = 0x00;

    buffer[i++] = (DNS_QTYPE_FIELD >> 8) & 0x00FF;
    buffer[i++] = DNS_QTYPE_FIELD & 0x00FF;

    buffer[i++] = (DNS_QCLASS_FIELD >> 8) & 0x00FF;
    buffer[i++] = DNS_QCLASS_FIELD & 0x00FF;

    *index = i;
}

int DnsProtocol::ResponseParse(const char *buffer, uint32_t len, std::string &ip) {
    uint32_t i = 0;

    i += 12;//跳过dns头

    while (buffer[i] != 0x00) {//跳过dns的question qname
        i += buffer[i] + 1;
    }

    if (i >= len) {
        return -1;
    }

    i += (1 + 4);//跳过question的qtype和qclass

    uint32_t rd_len = 0;
    uint32_t dns_count = 0;
    while (i < len) {
        /* name, type, class, ttl */
        i += (2 + 2 + 2 + 4);

        rd_len = (buffer[i++] << 8);
        rd_len |= buffer[i++];

        if (i >= len) {
            return -1;
        }

        if (rd_len == 4) {
            char dns[32] = {0};
            intToStr(&buffer[i], dns);
            ip = std::string(dns);
            dns_count++;
            break;//只简单取了第一个结果
        }

        i += rd_len;
    }

    if (dns_count == 0) {
        return -1;
    }

    return 0;
}

void DnsProtocol::intToStr(const char *input, char *output) {
    uint8_t idx = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t pos = 0;
    char temp[10] = {0};

    for (idx = 0; idx < 4; idx++) {
        i = 0;
        j = 0;
        pos = input[idx];
        memset(temp, 0, 10);
        do {
            temp[i++] = pos % 10 + '0';
        } while ((pos /= 10) > 0);

        do {
            output[--i + strlen(output)] = temp[j++];
        } while (i > 0);
        output[strlen(output)] = '.';
    }
    output[strlen(output) - 1] = 0x00;
}

} // namespace ftdwild30
