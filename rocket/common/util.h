#ifndef ROCKET_COMMON_UTIL_H
#define ROCKET_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>


namespace rocket{
    pid_t getPid(); //获取进程号
    pid_t getThreadId(); //获取线程号

    int64_t getNowMs(); //获取当前时间的毫秒数

    int32_t getInt32FromNetByte(const char* buf); //将网络字节序转换为一个Int型
}


#endif