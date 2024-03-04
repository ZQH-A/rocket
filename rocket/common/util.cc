#include "rocket/common/util.h"
#include <sys/syscall.h>


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
}