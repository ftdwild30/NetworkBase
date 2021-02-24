//
// Created by ftd.wild on 2020/9/5.
//

#include "f_engine.h"


#include "f_log.h"

namespace ftdwild30 {

Engine::Engine() : Thread() {
    fd_ = 0;
    io_ = new IoMultiplexing();
    pair_ = new SocketPair();
}

Engine::~Engine() {
    delete io_;
    delete pair_;
}

size_t Engine::Add(std::shared_ptr<Socket> socket) {
    mutex_.lock();
    size_t fd = ++fd_;
    sockets_.insert(std::make_pair(fd, socket));
    mutex_.unlock();
    pair_->Send("t", 1);//发送一字节任意数据，能唤醒即可

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
    //休眠一段时间
    io_->Sleep(kLoopTimeIntervalMs);

    mutex_.lock();
    if (sockets_.empty()) {
        mutex_.unlock();
        int ret = io_->Dispatch(false, kMaxSleepMs);
        if (ret < 0) {//异常错误，退出
            LOG_ERROR("Dispatch failed");
            return -1;
        }

        if (ret == 0) {//休眠到期无新增socket
            return 0;
        }

        /* 有新增socket或者退出消息等，进行处理(清空读缓冲数据） */
        bool read;
        bool write;
        if (io_->FdCheck(pair_->GetReadFd(), read, write) < 0) {
            LOG_ERROR("Fd check error");
            return -1;
        }
        if (read) {
            pair_->Readable();
        }
        return 0;
    }

    //取出所有待办项目
    SOCKET_MAP start_connect;
    SOCKET_MAP connecting;
    SOCKET_MAP connected;
    SOCKET_MAP disconnect;

    mutex_.lock();
    for (SOCKET_MAP::iterator it = sockets_.begin(); it != sockets_.end();) {
        if (it->second->Getstatus() == Socket::INIT) {
            ++it;
        } else if (it->second->Getstatus() == Socket::START_CONNECT) {
            start_connect.insert(*it);
            ++it;
        } else if (it->second->Getstatus() == Socket::CONNECTING) {
            start_connect.insert(*it);
            ++it;
        } else if (it->second->Getstatus() == Socket::CONNECTED) {
            connected.insert(*it);
            ++it;
        } else if (it->second->Getstatus() == Socket::DISCONNECTED) {
            disconnect.insert(*it);
            sockets_.erase(it++);
        } else {
            mutex_.unlock();
            return -1;
        }
    }
    mutex_.unlock();

    //无效连接关闭
    for (SOCKET_MAP::iterator it = disconnect.begin(); it != disconnect.end(); ++it) {
        if (io_->DeleteFd(it->second->GetFd() < 0)) {//删除无效链接
            LOG_ERROR("Delete fd failed");
        }
        it->second->RealDisconnect();
    }

    //处理连接事件
    for (SOCKET_MAP::iterator it = start_connect.begin(); it != start_connect.end(); ++it) {
        it->second->RealConnect();
    }

    //处理连接中的事件，确认是否超时
    for (SOCKET_MAP::iterator it = connecting.begin(); it != connecting.end(); ++it) {
        if (it->second->RealConnectCheck()) {

        } else {
            if (io_->AddFd(it->second->GetFd(), true, true) < 0) {
                LOG_ERROR("Add fd failed");
                it->second->RealDisconnect();
            }
        }
        connected.insert((*it));//连接未超时需要进行读写判断（和已连接的放在一起处理）
    }

    //无有效socket时，休眠一段时间
    if (connected.empty()) {
        return 0;
    }

    /* 连接成功和未超时的，都在io监听队列中，进行监听 */
    int ret = io_->Dispatch(false, kLoopTimeIntervalMs);
    if (ret < 0) {
        LOG_ERROR("Dispatch socket failed");
        return -1;
    }
    if (ret == 0) {
        //延时任务到期，无可读写的socket
        return 0;
    }
    bool read;
    bool write;
    for (SOCKET_MAP::iterator it = connected.begin(); it != connected.end(); ++it) {
        if (io_->FdCheck(it->second->GetFd(), read, write) < 0) {
            LOG_ERROR("Fd check error");
            return -1;
        }
        if (write) {//要先处理读事件
            it->second->RealWrite();
        }
        if (read) {//要先处理读事件
            it->second->RealRead();
        }
    }

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

void Engine::beforeStop() {
    pair_->Send("t", 1);//发送一字节任意数据，能唤醒即可
}

} // namespace ftdwild30
