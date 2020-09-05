//
// Created by ftd.wild on 2020/9/5.
//

#include "f_thread.h"

#include <unistd.h>
#include <thread>

#include "f_log.h"

namespace ftdwild30 {

Thread::Thread() {
    running_ = false;
    quit_ = true;
}

Thread::~Thread() {
    Stop();
}

int Thread::Start() {
    if (!quit_) {//有线程在运行了
        return 0;
    }

    if (beforeStart() < 0) {
        LOG_ERROR("Before start failed");
        return -1;
    }

    running_ = true;
    quit_ = false;
    std::thread t1(&Thread::thread, this);
    t1.detach();

    if (afterStart() < 0) {
        LOG_ERROR("After start failed");
        running_ = false;
        return -1;
    }

    LOG_INFO("Start thread success");
    return 0;
}

void Thread::Stop() {
    if (quit_) {
        return;
    }

    beforeStop();

    running_ = false;
    while (!quit_) {
        usleep(20 * 1000);
    }

    afterStop();
}


void Thread::thread() {
    LOG_INFO("Thread start");

    if (beforeThread() < 0) {
        LOG_ERROR("Before thread failed");
        running_ = false;
    }

    while (running_) {
        if (readThread() < 0) {
            running_ = false;
        }
    }

    afterThread();

    quit_ = true;

    LOG_INFO("Thread stop");
}


} // namespace ftdwild30
