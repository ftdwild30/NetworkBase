//
// Created by ftd.wild on 2020/9/5.
//

#include "f_log.h"

#include <stdarg.h>
#include <stdio.h>

namespace ftdwild30 {

#define RESET ("\033[0m")
#define RED ("\033[31m")
#define GREEN ("\033[32m")
#define YELLOW ("\033[33m")


static const char *LOG_LEVEL_NAME[] = {"ERROR  ", "WARN   ", "INFO   ", "DEBUG  ", "VERBOSE"};
static const char *LOG_LEVEL_COLOR[] = {RED, RED, YELLOW, GREEN, GREEN};

void log_cb(unsigned int level, const char *file_name, int line, const char *format, ...) {
    //TODO,打印权限控制
    if (level > sizeof(LOG_LEVEL_NAME) / sizeof(LOG_LEVEL_NAME[0])) {
        level = 0;
    }

    fprintf(stdout, "%s[%s] (%s:%d) :%s", LOG_LEVEL_COLOR[level], LOG_LEVEL_NAME[level], file_name, line, RESET);
    va_list arg;
    va_start(arg, format);
    vfprintf(stdout, format, arg);
    va_end(arg);
    fprintf(stdout, "\n");
    fflush(stdout);
}

} // namespace ftdwild30
