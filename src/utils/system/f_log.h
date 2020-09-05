//
// Created by ftd.wild on 2020/9/5.
//

#ifndef NETWORK_BASE_F_LOG_H
#define NETWORK_BASE_F_LOG_H

#include <string.h>

/**
 * @notice : 这是一个简易的日志打印实现。
 *           若需要添加日志打印的各项功能，推荐在遵守zlog开源协议的前提下，使用zlog。
 *
 */

namespace ftdwild30 {

enum LOG_LEVEL {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE,
};

void log_cb(unsigned int level, const char *file_name, int line, const char *format, ...);

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_ERROR(args...) log_cb(LOG_LEVEL_ERROR, __FILENAME__, __LINE__, args)
#define LOG_WARN(args...) log_cb(LOG_LEVEL_WARN, __FILENAME__, __LINE__, args)
#define LOG_INFO(args...) log_cb(LOG_LEVEL_INFO, __FILENAME__, __LINE__, args)
#define LOG_DEBUG(args...) log_cb(LOG_LEVEL_DEBUG, __FILENAME__, __LINE__, args)
#define LOG_VERBOSE(args...) log_cb(LOG_LEVEL_VERBOSE, __FILENAME__, __LINE__, args)


} // namespace ftdwild30


#endif //NETWORK_BASE_F_LOG_H
