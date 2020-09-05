//
// Created by ftd.wild on 2020/9/5.
//

#include "f_engine.h"


#include "f_log.h"

namespace ftdwild30 {

Engine::Engine() : Thread() {
    fd_ = 0;
    io_ = new IoMultiplexing();
}

Engine::~Engine() {
    delete io_;
}

size_t Engine::Add(std::shared_ptr<Socket> socket) {
    mutex_.lock();
    size_t fd = ++fd_;
    sockets_.insert(std::make_pair(fd, socket));
    mutex_.unlock();

    return fd;
}

bool Engine::Query(size_t fd, std::shared_ptr<Socket> &socket) {
    mutex_.lock();

    SOCKET_MAP::iterator it = sockets_.find(fd);
    if (it == sockets_.end()) {
        mutex_.unlock();
        return false;
    }
    socket = it->second;

    mutex_.unlock();

    return true;
}

int Engine::readThread() {
    io_->Clear();

    //取出所有待办项目
    SOCKET_MAP connecting_sockets;
    SOCKET_MAP connected_sockets;
    SOCKET_MAP disconnect_sockets;

    mutex_.lock();
    for (SOCKET_MAP::iterator it = sockets_.begin(); it != sockets_.end();) {
        if (it->second->Getstatus() == Socket::INIT) {
            ++it;
        } else if (it->second->Getstatus() == Socket::START_CONNECTING) {
            connecting_sockets.insert(*it);
            ++it;
        } else if (it->second->Getstatus() == Socket::CONNECTED) {
            connected_sockets.insert(*it);
            ++it;
        } else if (it->second->Getstatus() == Socket::DISCONNECTED) {
            disconnect_sockets.insert(*it);
            sockets_.erase(it++);
        } else {
            mutex_.unlock();
            return -1;
        }
    }
    mutex_.unlock();

    //无效连接关闭
    for (SOCKET_MAP::iterator it = disconnect_sockets.begin(); it != disconnect_sockets.end(); ++it) {
        it->second->RealDisconnect();
    }

    //处理连接事件
    for (SOCKET_MAP::iterator it = connecting_sockets.begin(); it != connecting_sockets.end(); ++it) {
        it->second->RealConnect();
    }

    //无有效socket时，休眠一段时间（也可以不休眠，多占用CPU）
    if (connected_sockets.empty()) {
        io_->Dispatch(false, kLoopTimeIntervalMs);
        return 0;
    }

    //处理已连接socket的读写事件
    for (SOCKET_MAP::iterator it = connected_sockets.begin(); it != connected_sockets.end(); ++it) {
        io_->AddFd(it->second->GetFd(), true, true);
    }
    int ret = io_->Dispatch(false, kLoopTimeIntervalMs);
    if (ret < 0) {
        LOG_ERROR("Dispatch socket failed");
        return -1;
    }
    if (ret == 0) {
        //延时任务到期，无可读写的socket
        return 0;
    }
    for (SOCKET_MAP::iterator it = connected_sockets.begin(); it != connected_sockets.end(); ++it) {
        if (io_->IsWritable(it->second->GetFd())) {
            it->second->RealWrite();
        }
        if (io_->IsReadable(it->second->GetFd())) {
            it->second->RealRead();
        }
    }

    //休眠一段时间（也可以不休眠）
    io_->Clear();
    io_->Dispatch(false, kLoopTimeIntervalMs);

    return 0;
}

int Engine::afterThread() {
    //线程退出时，关闭所有socket
    mutex_.lock();
    SOCKET_MAP disconnect_sockets;
    sockets_.swap(disconnect_sockets);
    mutex_.unlock();

    for (SOCKET_MAP::iterator it = disconnect_sockets.begin(); it != disconnect_sockets.end(); ++it) {
        it->second->Close();
        it->second->RealDisconnect();
    }

    return 0;
}

} // namespace ftdwild30
