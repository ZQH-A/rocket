#ifndef ROCKET_NET_IO_THREAD_GROUP_H
#define ROCKET_NET_IO_THREAD_GROUP_H

#include <vector>
#include "rocket/common/log.h"
#include "rocket/net/io_thread.h"

namespace rocket{

    class IOThreadGroup
    {
    private:
        int m_size {0};
        std::vector<IOThread*> m_io_thread_groups;

        int m_index {0}; //轮询

    public:
        IOThreadGroup(int size);
        ~IOThreadGroup();

        void start(); //控制所有IO线程loop循环的开始

        void join();  //主线程等待创建的线程
        IOThread* getIOThread(); //获取一个可用的io线程
    };
    

}



#endif