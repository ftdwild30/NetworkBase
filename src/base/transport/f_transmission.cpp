//
// Created by ftd.wild on 2020/9/6.
//

#include "f_transmission.h"


namespace ftdwild30 {

Transmission::Transmission(std::shared_ptr<SocketHandler> handler) : Socket() {
    handler_ = handler;
    send_ = new AutoGrowBuffer(kSendBufLen);
    read_ = new AutoGrowBuffer(kReadBufLen);
}

Transmission::~Transmission() {
    delete send_;
    delete read_;
}

ssize_t Transmission::Send(const char *buf, size_t len) {
    return send_->AddTailData(buf, len);
}

void Transmission::OnConnect() {
    handler_->OnConnect();
}

void Transmission::OnDisconnect() {
    handler_->OnDisconnect();
}

ssize_t Transmission::OnRead(const char *data, size_t len, bool finish) {
    if (read_->AddTailData(data, len) < 0) {
        return -1;
    }
    if (finish) {
//        read_->PopHeadData([this](char *pop_data, size_t pop_len) {return this->readBufferPopData(pop_data, pop_len);});
        read_->PopHeadData(std::bind(&Transmission::readBufferPopData, this, std::placeholders::_1, std::placeholders::_2));
    }

    return len;
}

ssize_t Transmission::OnWrite() {
    if (send_->GetSize()) {
        return send_->PopHeadData(std::bind(&Transmission::writeBufferPopData, this, std::placeholders::_1, std::placeholders::_2));
    }

    handler_->OnWrite();
    return 0;
}

ssize_t Transmission::writeBufferPopData(const char *buf, size_t len) {
    return send(buf, len);
}

ssize_t Transmission::readBufferPopData(const char *buf, size_t len) {
    return handler_->OnData(buf, len);
}

} // namespace ftdwild30