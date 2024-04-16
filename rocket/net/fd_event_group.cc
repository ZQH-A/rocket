#include "rocket/net/fd_event_group.h"
#include "rocket/common/mutex.h"
#include "rocket/common/log.h"

namespace rocket{

    static FdEventGroup* g_fd_event_group  = NULL; //全局单例

    FdEventGroup* FdEventGroup::GetFdEventGroup()
    {
        if(g_fd_event_group != NULL)
        {
            return g_fd_event_group;
        }
        g_fd_event_group = new FdEventGroup(128);

        return g_fd_event_group;
    }
    FdEventGroup::FdEventGroup(int size) : m_size(size)
    {
        for(int i=0;i < m_size; ++i)
        {
            m_fd_group.push_back(new FdEvent(i));
        }
    }
    FdEventGroup::~FdEventGroup()
    {
        for(int i =0;i < m_size;++i)
        {
            if(m_fd_group[i] != NULL)
            {
                delete m_fd_group[i];
                m_fd_group[i] = NULL;
            }
        }
    }

    FdEvent* FdEventGroup::getFdEvent(int fd) //返回fd对应的FdEvent
    {
        ScopeMutext<Mutex> lock(m_mutex);
        if((size_t)fd < m_fd_group.size())  //当前的fd在 Group的范围内
        {
            return m_fd_group[fd];
        }

        //如果没在，则扩种group的范围
        int new_size = int(fd *1.5);
        for(int i= m_fd_group.size();i<new_size;++i)
        {
            m_fd_group.push_back(new FdEvent(i));
        }

        return m_fd_group[fd];
    }

    
}