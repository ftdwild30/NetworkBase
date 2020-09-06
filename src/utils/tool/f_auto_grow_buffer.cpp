//
// Created by ftd.wild on 2020/9/6.
//

#include "f_auto_grow_buffer.h"


namespace ftdwild30 {

AutoGrowBuffer::AutoGrowBuffer(size_t default_len) {
    true_len_ = default_len > kMinLen ? default_len : kMinLen;
    true_len_ = true_len_ < kMaxLen ? true_len_ : kMaxLen;
    current_begin_position_ = 0;
    current_end_position_ = 0;
    true_buf_ = new char[true_len_];
}

AutoGrowBuffer::~AutoGrowBuffer() {
    delete [] true_buf_;
}

ssize_t AutoGrowBuffer::AddTailData(const char *data, size_t len) {
    if (!(data && len)) {
        return 0;
    }
    if (len > kMaxLen) {
        return -1;
    }

    mutex_.lock();
    //先判断尾部是否有空余空间
    if (current_end_position_ + len <= true_len_) {

    } else if ((true_len_ - current_end_position_ + current_begin_position_) >= len) {//尾部空间足够，整体内存前移
        memmove(true_buf_, true_buf_ + current_begin_position_, current_end_position_ - current_begin_position_);
        current_end_position_ -= current_begin_position_;
        current_begin_position_ = 0;
    } else {//总空间不够，重新申请
        size_t  current_len = current_end_position_ - current_begin_position_;
        size_t new_len = GetReallocLen(current_len + len);
        char *new_buf = new char[new_len];
        memcpy(new_buf, true_buf_ + current_begin_position_, current_len);
        delete [] true_buf_;
        true_buf_ = new_buf;
        true_len_ = new_len;
        current_begin_position_ = 0;
        current_end_position_ = current_len;
    }

    //拷贝传入的数据
    memcpy(true_buf_ + current_end_position_, data, len);
    current_end_position_ += len;
    mutex_.unlock();
    return len;
}

ssize_t AutoGrowBuffer::PopHeadData(std::function<ssize_t(char *, size_t)> pop_data) {
    mutex_.lock();
    size_t pop_len_expert = current_end_position_ - current_begin_position_;
    ssize_t pop_len = pop_data(true_buf_ + current_begin_position_, pop_len_expert);
    if (pop_len < 0 || pop_len > pop_len_expert) {
        mutex_.unlock();
        return -1;
    }
    current_begin_position_ += pop_len;
    mutex_.unlock();
    return pop_len;
}

size_t AutoGrowBuffer::GetSize() {
    mutex_.lock();
    size_t get_size = current_end_position_ - current_begin_position_;
    mutex_.unlock();
    return get_size;
}

size_t AutoGrowBuffer::GetReallocLen(size_t len) {
    //2次幂对齐
    int size_pow = 0;
    while (len > 0) {
        len /= 2;
        size_pow++;
    }
    return 1 << size_pow;
}

} // namespace ftdwild30
