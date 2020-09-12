//
// Created by ftd.wild on 2020/9/12.
//

#include "f_transmission_security.h"

#include "f_log.h"

namespace ftdwild30 {


TransmissionSecurity::TransmissionSecurity(std::shared_ptr<SocketHandler> handler) : Socket() {
    handler_ = handler;

    init_ = false;

    ssl_ = new mbedtls_ssl_context;
    conf_ = new mbedtls_ssl_config;
    crt_ = new mbedtls_x509_crt;
    entropy_ = new mbedtls_entropy_context;
    ctr_drbg_ = new mbedtls_ctr_drbg_context;

    tls_send_ = new AutoGrowBuffer(kTlsSendLen);
    tls_read_ = new AutoGrowBuffer(kTlsReadLen);
    source_read_ = new AutoGrowBuffer(kTlsSendLen);
    source_send_ = new AutoGrowBuffer(kTlsReadLen);

    tls_read_cache_ = new char[kTlsCacheLen];

    read_temp_ = nullptr;
    read_temp_len_ = 0;
}

TransmissionSecurity::~TransmissionSecurity() {
    if (init_) {
        mbedtlsResourceDestroy();
    }
    delete ssl_;
    delete conf_;
    delete crt_;
    delete entropy_;
    delete ctr_drbg_;
    delete tls_send_;
    delete tls_read_;
    delete source_read_;
    delete source_send_;
    delete [] tls_read_cache_;
}

int TransmissionSecurity::SetSecurityLicence(const char *ca, size_t ca_len) {
    if (init_) {
        return 0;
    }

    mbedtlsResourceCreate();

    mbedtls_entropy_init(entropy_);
    int ret = mbedtls_ctr_drbg_seed(ctr_drbg_, mbedtls_entropy_func,entropy_, nullptr, 0);
    if (ret != 0) {
        LOG_ERROR("mbedtls_ctr_drbg_seed failed, ret = %d", ret);
        mbedtlsResourceDestroy();
        return -1;
    }

    ret = mbedtls_x509_crt_parse(crt_, (const unsigned char *)ca, ca_len + 1);
    if (ret < 0) {
        LOG_ERROR("mbedtls_ctr_drbg_seed failed, ret = %d", ret);
        mbedtlsResourceDestroy();
        return -1;
    }

    ret = mbedtls_ssl_config_defaults(conf_, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {//todo,需要支持udp
        LOG_ERROR("mbedtls_ssl_config_defaults failed, ret = %d", ret);
        mbedtlsResourceDestroy();
        return -1;
    }

    mbedtls_ssl_conf_authmode(conf_, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(conf_, crt_, nullptr);
    mbedtls_ssl_conf_rng(conf_, mbedtls_ctr_drbg_random, ctr_drbg_);

    ret = mbedtls_ssl_setup(ssl_, conf_);
    if (ret != 0) {
        LOG_ERROR("mbedtls_ssl_setup failed, ret = %d", ret);
        mbedtlsResourceDestroy();
        return -1;
    }

    mbedtls_ssl_set_bio(ssl_, this, mbedtlsNetSend, mbedtlsNetRecv, nullptr);

    LOG_INFO("SetSecurityLicence success");
    init_ = true;

    return 0;
}

ssize_t TransmissionSecurity::Send(const char *buf, size_t len) {
    if (!init_) {
        return -1;
    }

    return source_send_->AddTailData(buf, len);
}

void TransmissionSecurity::OnConnect() {
    handler_->OnConnect();
}

void TransmissionSecurity::OnDisconnect() {
    handler_->OnDisconnect();
}

ssize_t TransmissionSecurity::OnRead(const char *data, size_t len, bool finish) {
    if (!init_) {
        return -1;
    }

    //先写入tls读缓冲
    if (tls_read_->AddTailData(data, len) < 0) {
        return -1;
    }
    if (!finish) {
        return len;
    }

    //判断握手是否完成
    int ret = mbedtls_ssl_handshake(ssl_);
    if (ret != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            LOG_ERROR("mbedtls_ssl_handshake failed, ret = %d", ret);
            return -1;
        }
        return len;
    }

    //完成握手，解密数据
    bool flag = true;
    while (flag) {
        ret = mbedtls_ssl_read(ssl_, (unsigned char *)tls_read_cache_, kTlsCacheLen);
        //无数据可处理时，退出等下一次调用
        if (ret <= 0) {
            if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
                flag = false;
                continue;
            } else {
                LOG_ERROR("mbedtls_ssl_read failed, ret = %d", ret);
                return -1;
            }
        }
        //解密数据放入原始数据缓冲
        if (source_read_->AddTailData(tls_read_cache_, ret) < 0) {
            return -1;
        }
    }

    //原始数据对外pop出
    if (source_read_->PopHeadData(std::bind(&TransmissionSecurity::sourceReadBufferPopData, this, std::placeholders::_1, std::placeholders::_2)) < 0) {
        return -1;
    }

    return len;
}

ssize_t TransmissionSecurity::OnWrite() {
    if (!init_) {
        return -1;
    }

    //判断握手是否完成
    int ret = mbedtls_ssl_handshake(ssl_);
    if (ret != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            LOG_ERROR("mbedtls_ssl_handshake failed, ret = %d", ret);
            return -1;
        }
        return 0;
    }

    //取出原始缓冲区的数据并加密
    if (source_send_->GetSize()) {
        ssize_t result = source_send_->PopHeadData(std::bind(&TransmissionSecurity::sourceSendBufferPopData, this, std::placeholders::_1, std::placeholders::_2));
        if (result < 0) {
            return -1;
        }
    }

    //将加密的数据进行发送
    if (tls_send_->GetSize()) {
        ssize_t result = tls_send_->PopHeadData(std::bind(&TransmissionSecurity::tlsSendBufferPopData, this, std::placeholders::_1, std::placeholders::_2));
        if (result < 0) {
            return -1;
        }
    }

    if (!(tls_send_->GetSize() && tls_send_->GetSize())) {
        handler_->OnWrite();
    }

    return 0;
}

void TransmissionSecurity::mbedtlsResourceCreate() {
    mbedtls_ssl_init(ssl_);
    mbedtls_ssl_config_init(conf_);
    mbedtls_x509_crt_init(crt_);
    mbedtls_ctr_drbg_init(ctr_drbg_);
    mbedtls_entropy_init(entropy_);
}

void TransmissionSecurity::mbedtlsResourceDestroy() {
    mbedtls_x509_crt_free(crt_);
    mbedtls_ssl_free(ssl_);
    mbedtls_ssl_config_free(conf_);
    mbedtls_entropy_free(entropy_);
    mbedtls_ctr_drbg_free(ctr_drbg_);
}

ssize_t TransmissionSecurity::sourceSendBufferPopData(const char *buf, size_t len) {
    int ret = mbedtls_ssl_write(ssl_, (unsigned char *)buf, len);
    if (ret <=0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            LOG_ERROR("mbedtls_ssl_write failed, ret = %d", ret);
            return -1;
        } else {
            return 0;
        }
    }
    return ret;
}

ssize_t TransmissionSecurity::sourceReadBufferPopData(const char *buf, size_t len) {
    return handler_->OnData(buf, len);
}

ssize_t TransmissionSecurity::tlsSendBufferPopData(const char *buf, size_t len) {
    return send(buf, len);
}

ssize_t TransmissionSecurity::tlsReadBufferPopData(const char *buf, size_t len) {
    size_t copy_len = read_temp_len_ > len ? len:read_temp_len_;
    memcpy(read_temp_, buf, copy_len);
    return copy_len;
}

int TransmissionSecurity::mbedtlsNetRecv(void *ctx, unsigned char *buf, size_t len) {
    if (!ctx) {
        return -1;
    }

    TransmissionSecurity *ts = (TransmissionSecurity *)ctx;
    if (ts->tls_read_->GetSize() < len) {
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    ts->read_temp_ = buf;
    ts->read_temp_len_ = len;
    int ret = ts->tls_read_->PopHeadData(std::bind(&TransmissionSecurity::tlsReadBufferPopData, ts, std::placeholders::_1, std::placeholders::_2));
    if (ret < 0) {
        return -1;
    }

    return ret;
}

int TransmissionSecurity::mbedtlsNetSend(void *ctx, const unsigned char *buf, size_t len) {
    if (!ctx) {
        return -1;
    }

    TransmissionSecurity *ts = (TransmissionSecurity *)ctx;
    if (ts->send((char *)buf, len) < 0) {
        return -1;
    }
    return len;
}

} // namespace ftdwild30
