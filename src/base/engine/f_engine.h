//
// Created by ftd.wild on 2020/9/5.
//

#ifndef NETWORK_BASE_F_ENGINE_H
#define NETWORK_BASE_F_ENGINE_H

#include <memory>
#include <mutex>
#include <map>

#include "f_socket.h"
#include "f_socket_pair.h"
#include "f_thread.h"
#include "f_io_multiplexing.h"


namespace ftdwild30 {


class Engine : public Thread {
public:
    Engine();
    virtual ~Engine();

    size_t Add(std::shared_ptr<Socket> socket);

    bool Query(size_t fd, std::shared_ptr<Socket> &socket);

private:
    virtual int readThread();
    virtual int afterThread();
    virtual void beforeStop();

private:
    size_t fd_;
    IoMultiplexing *io_;
    SocketPair *pair_;//有新socket通知用，降低无socket时的CPU占用
    std::mutex mutex_;
    typedef std::map<size_t, std::shared_ptr<Socket>> SOCKET_MAP;
    SOCKET_MAP sockets_;

private:
    static const int kLoopTimeIntervalMs = 30;
    static const size_t kMaxSleepMs = 60000;
};

} // namespace ftdwild30


#endif //NETWORK_BASE_F_ENGINE_H
