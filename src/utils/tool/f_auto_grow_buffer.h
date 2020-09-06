//
// Created by ftd.wild on 2020/9/6.
//

#ifndef NETWORK_BASE_F_AUTO_GROW_BUFFER_H
#define NETWORK_BASE_F_AUTO_GROW_BUFFER_H

#include <unistd.h>

#include <functional>
#include <mutex>

namespace ftdwild30 {


class AutoGrowBuffer {
public:
    explicit AutoGrowBuffer(size_t default_len);
    ~AutoGrowBuffer();

    /* 向队列尾添加数据 */
    ssize_t AddTailData(const char *data, size_t len);

    ssize_t PopHeadData(std::function<ssize_t(char *, size_t)> pop_data);

    size_t GetSize();

private:
    static size_t GetReallocLen(size_t len);

private:
    //TODO,内存申请优化
    char *true_buf_;
    size_t true_len_;
    size_t current_begin_position_;
    size_t current_end_position_;
    std::mutex mutex_;

private:
    static const size_t kMaxLen = 1024 * 1024;
    static const size_t kMinLen = 4 * 1024;
};


} // namespace ftdwild30

#endif //NETWORK_BASE_F_AUTO_GROW_BUFFER_H
