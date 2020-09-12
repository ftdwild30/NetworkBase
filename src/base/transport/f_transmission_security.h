//
// Created by ftd.wild on 2020/9/12.
//

#ifndef NETWORK_BASE_F_TRANSMISSION_SECURITY_H
#define NETWORK_BASE_F_TRANSMISSION_SECURITY_H

#include <memory>

#include "f_socket_handler.h"
#include "f_socket.h"
#include "f_auto_grow_buffer.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ssl.h"

namespace ftdwild30 {

class TransmissionSecurity : public Socket {
public:
    explicit TransmissionSecurity(std::shared_ptr<SocketHandler> handler);
    virtual ~TransmissionSecurity();

    int SetSecurityLicence(const char *ca, size_t ca_len);

    virtual ssize_t Send(const char *buf, size_t len);

private:
    //当连接成功时被调用
    virtual void OnConnect();
    //当连接断开时被调用
    virtual void OnDisconnect();
    //当连接读取到数据时被调用
    virtual ssize_t OnRead(const char *data, size_t len, bool finish);
    //当连接可写时被调用
    virtual ssize_t OnWrite();

private:
    void mbedtlsResourceCreate();
    void mbedtlsResourceDestroy();
    ssize_t sourceSendBufferPopData(const char *buf, size_t len);
    ssize_t sourceReadBufferPopData(const char *buf, size_t len);
    ssize_t tlsSendBufferPopData(const char *buf, size_t len);
    ssize_t tlsReadBufferPopData(const char *buf, size_t len);

    static int mbedtlsNetRecv(void *ctx, unsigned char *buf, size_t len);
    static int mbedtlsNetSend(void *ctx, const unsigned char *buf, size_t len);


private:
    bool init_;
    std::shared_ptr<SocketHandler> handler_;

    /* mbedtls的一些配置 */
    mbedtls_ssl_context *ssl_;
    mbedtls_ssl_config *conf_;
    mbedtls_x509_crt *crt_;
    mbedtls_entropy_context *entropy_;
    mbedtls_ctr_drbg_context *ctr_drbg_;

    /* tls的缓冲 */
    AutoGrowBuffer *tls_send_;
    AutoGrowBuffer *tls_read_;
    AutoGrowBuffer *source_read_;
    AutoGrowBuffer *source_send_;
    char *tls_read_cache_;

    /* mbedtls读临时记录用的 */
    unsigned char *read_temp_;
    size_t read_temp_len_;

private:
    static const int kTlsCacheLen = 16384;
    static const int kTlsSendLen = 16384;
    static const int kTlsReadLen = 16384;
};

} // namespace ftdwild30

#endif //NETWORK_BASE_F_TRANSMISSION_SECURITY_H
