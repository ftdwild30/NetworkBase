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

int IoMultiplexing::AddFd(int fd, bool read, bool write) {
    if (fd > fd_max_) {
        fd_max_ = fd;
    }

    int fd_status = 0;
    if (read) {
        fd_status |= kRead;
    }
    if (write) {
        fd_status |= kWrite;
    }

    map_.insert(std::make_pair(fd, fd_status));

    return 0;
}

int IoMultiplexing::DeleteFd(int fd) {
    std::map<int, int>::iterator it = map_.find(fd);
    if (it == map_.end()) {
        return -1;
    }

    map_.erase(fd);
    return 0;
}

int IoMultiplexing::Dispatch(bool endless, size_t time_out_ms) {
    FD_ZERO(read_set_);
    FD_ZERO(write_set_);

    for(std::map<int, int>::const_iterator it = map_.begin(); it != map_.end(); ++it) {
        if (it->second & kRead) {
            FD_SET(it->first, read_set_);
        }
        if (it->second & kWrite) {
            FD_SET(it->first, write_set_);
        }
    }

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


int IoMultiplexing::Sleep(size_t time_ms) {
    struct timeval tv = {0};
    tv.tv_sec = time_ms / 1000;
    tv.tv_usec = (time_ms - tv.tv_sec * 1000) * 1000;
    return select(0, nullptr, nullptr, nullptr, &tv);
}

int IoMultiplexing::FdCheck(int fd, bool &read_able, bool &write_able) {
    if (FD_ISSET(fd, read_set_)) {
        read_able = true;
    } else {
        read_able = false;
    }

    if (FD_ISSET(fd, write_set_)) {
        write_able = true;
    } else {
        write_able = false;
    }

    return 0;
}

} // namespace ftdwild30
