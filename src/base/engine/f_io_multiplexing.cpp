//
// Created by ftd.wild on 2020/9/5.
//

#include "f_io_multiplexing.h"
namespace ftdwild30 {

IoMultiplexing::IoMultiplexing() {
    read_set_ = new fd_set;
    write_set_ = new fd_set;
    fd_max_ = 0;
}

IoMultiplexing::~IoMultiplexing() {
    Clear();
    delete read_set_;
    delete write_set_;
}

void IoMultiplexing::AddFd(int fd, bool read, bool write) {
    if (fd > fd_max_) {
        fd_max_ = fd;
    }
    if (read) {
        FD_SET(fd, read_set_);
    }
    if (write) {
        FD_SET(fd, write_set_);
    }
}

int IoMultiplexing::Dispatch(bool endless, size_t time_out_ms) {
    struct timeval tv = {0};
    tv.tv_sec = time_out_ms / 1000;
    tv.tv_usec = (time_out_ms - tv.tv_sec * 1000) * 1000;
    if (fd_max_ == 0) {
        return select(0, nullptr, nullptr, nullptr, endless ? nullptr : &tv);
    } else {
        return select(fd_max_ + 1, read_set_, write_set_, nullptr, endless ? nullptr : &tv);
    }
}

void IoMultiplexing::Clear() {
    FD_ZERO(write_set_);
    FD_ZERO(read_set_);
    fd_max_ = 0;
}

bool IoMultiplexing::IsReadable(int fd) {
    return FD_ISSET(fd, read_set_);
}

bool IoMultiplexing::IsWritable(int fd) {
    return FD_ISSET(fd, write_set_);
}

} // namespace ftdwild30
