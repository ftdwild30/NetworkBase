//
// Created by ftd.wild on 2020/9/20.
//

#include "f_task_queue.h"

#include "f_time.h"

namespace ftdwild30 {

TaskEntry::TaskEntry() {
    forward_time_ = 0;
}

TaskEntry::~TaskEntry() {

}

void TaskEntry::Run() {

}

void TaskEntry::SetDelayTime(size_t delay_ms) {
    forward_time_ = Time::GetSystemTimeMs() + delay_ms;
}

size_t TaskEntry::GetForwardTime() const {
    return forward_time_;
}

TaskQueue::TaskQueue() : Thread() {
    pair_ = new SocketPair();
    io_ = new IoMultiplexing();
    sleep_time_ = 0;
}

TaskQueue::~TaskQueue() {
    delete pair_;
    delete io_;
}

void TaskQueue::AddTask(std::shared_ptr<TaskEntry> task) {
    mutex_.lock();
    tasks_.push_back(task);
    mutex_.unlock();
    pair_->Send("t", 1);
}

int TaskQueue::doTask() {
    size_t system_time = Time::GetSystemTimeMs();
    TASK_LIST task_copy;
    size_t forward_time = 0;

    mutex_.lock();
    for (TASK_LIST::iterator it = tasks_.begin(); it != tasks_.end();) {
        //取出已经到期的任务
        if ((*it)->GetForwardTime() <= system_time) {
            task_copy.push_back(*it);
            tasks_.erase(it++);
        } else {
            //计算最快到期的任务事项
            if (forward_time == 0) {
                forward_time = (*it)->GetForwardTime();
            } else if (forward_time > (*it)->GetForwardTime()) {
                forward_time = (*it)->GetForwardTime();
            }
            ++it;
        }
    }

    if (tasks_.empty()) {
        sleep_time_ = kMaxSleepMs;
    } else {
        //有待办任务，计算最短需要休眠的时间
        if (system_time > forward_time) {
            mutex_.unlock();
            return -1;
        }
        sleep_time_ = forward_time - system_time;
    }
    mutex_.unlock();

    //执行任务
    for (TASK_LIST::iterator it = task_copy.begin(); it != task_copy.end(); ++ it) {
        (*it)->Run();
    }

    return 0;
}

int TaskQueue::beforeStart() {
    return pair_->Start();
}

void TaskQueue::beforeStop() {
    pair_->Send("e", 1);
}

int TaskQueue::readThread() {
    if (pair_->GetCloseStatus()) {
        return -1;
    }

    io_->Clear();
    io_->AddFd(pair_->GetReadFd(), true, false);

    int ret = io_->Dispatch(false, sleep_time_);

    if (ret < 0) {
        return -1;
    }

    if (ret == 0) {//有任务到期
        return doTask();
    }

    //有新增任务
    if (!io_->IsReadable(pair_->GetReadFd())) {
        return -1;
    }
    pair_->Readable();

    return doTask();
}

int TaskQueue::afterThread() {
    return doTask();
}

void TaskQueue::afterStop() {
    pair_->Stop();
}

} // namespace ftdwild30
