#ifndef ROCKET_COMMON_UTIL_H
#define ROCKET_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>


namespace rocket{
    pid_t getPid(); //获取进程号
    pid_t getThreadId(); //获取线程号
}


#endif