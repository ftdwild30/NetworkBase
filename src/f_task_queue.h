//
// Created by ftd.wild on 2020/9/20.
//

#ifndef NETWORK_BASE_F_TASK_QUEUE_H
#define NETWORK_BASE_F_TASK_QUEUE_H

#include "f_thread.h"
#include "f_socket_pair.h"
#include "f_io_multiplexing.h"

#include <memory>
#include <list>
#include <mutex>

namespace ftdwild30 {




class TaskEntry {
public:
    TaskEntry();
    virtual ~TaskEntry();

    virtual void Run() = 0;

    void SetDelayTime(size_t delay_ms);

private:
    size_t GetForwardTime() const;
    friend class TaskQueue;

private:
    size_t forward_time_;
};

typedef std::list<std::shared_ptr<TaskEntry>> TASK_LIST;
class TaskQueue : public Thread {
public:
    TaskQueue();
    virtual ~TaskQueue();

    void AddTask(std::shared_ptr<TaskEntry> task);

private:
    int doTask();

private:
    virtual int beforeStart();
    virtual void beforeStop();
    virtual int readThread();
    virtual int afterThread();
    virtual void afterStop();

private:
    size_t sleep_time_;
    SocketPair *pair_;
    IoMultiplexing *io_;
    std::mutex mutex_;
    TASK_LIST tasks_;

private:
    static const size_t kMaxSleepMs = 60000;
};




} // namespace ftdwild30

#endif //NETWORK_BASE_F_TASK_QUEUE_H
