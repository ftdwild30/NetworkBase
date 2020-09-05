//
// Created by ftd.wild on 2020/9/5.
//

#ifndef NETWORK_BASE_F_ENGINE_H
#define NETWORK_BASE_F_ENGINE_H

#include <memory>
#include <mutex>
#include <map>

#include "f_socket.h"
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

private:
    size_t fd_;
    IoMultiplexing *io_;
    std::mutex mutex_;
    typedef std::map<size_t, std::shared_ptr<Socket>> SOCKET_MAP;
    SOCKET_MAP sockets_;

private:
    static const int kLoopTimeIntervalMs = 30;
};

} // namespace ftdwild30


#endif //NETWORK_BASE_F_ENGINE_H
