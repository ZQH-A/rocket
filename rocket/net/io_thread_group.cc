#include "rocket/net/io_thread_group.h"


namespace rocket{


    IOThreadGroup::IOThreadGroup(int size):m_size(size)
    {
        m_io_thread_groups.resize(size);

        for(int i=0; i < size; ++i)
        {
            m_io_thread_groups[i] = new IOThread();
        }
    }
    IOThreadGroup::~IOThreadGroup()
    {

    }

    void IOThreadGroup::start() //控制所有IO线程loop循环的开始
    {
        for(int i = 0; i < m_size; ++i)
        {
            m_io_thread_groups[i]->start();
        }
    }

    void IOThreadGroup::join()  //主线程等待创建的线程
    {
        for(int i = 0; i < m_size; ++i)
        {
            m_io_thread_groups[i]->join();
        }
    }

    IOThread* IOThreadGroup::getIOThread() //获取一个可用的io线程
    {
        if(m_index == m_size || m_index == -1)
        {
            m_index = 0;
        }
        return m_io_thread_groups[m_index++];
    }
}