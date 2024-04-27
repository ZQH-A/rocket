#include "rocket/common/util.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>

namespace rocket{
    static int g_pid = 0;
    static thread_local int g_thread_id = 0; //线程局部变量

    pid_t getPid() //获取进程号
    {
        if (g_pid !=0)
        {
            return g_pid;
        }
        return getpid();
    }
    pid_t getThreadId() //获取线程号
    {
        if(g_thread_id != 0)
        {
            return g_thread_id;
        }
        return syscall(SYS_gettid);
    }


    int64_t getNowMs() //获取当前时间的毫秒数
    {
        timeval val;
        gettimeofday(&val,NULL);

        return val.tv_sec *1000 + val.tv_usec / 1000;
    }


    int32_t getInt32FromNetByte(const char* buf) //将网络字节序转换为一个Int型
    {
        int32_t re;
        memcpy(&re,buf,sizeof(re));  //读取了四个字节

        return ntohl(re); //将网络字节序转为主机字节序
    }
}