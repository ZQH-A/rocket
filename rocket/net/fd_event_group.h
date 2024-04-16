#ifndef ROCKET_NET_FD_EVENT_GROUP_H
#define ROCKET_NET_FD_EVENT_GROUP_H

#include "rocket/net/fd_event.h"
#include <vector>
#include "rocket/common/mutex.h"


namespace rocket{
    class FdEventGroup  //管理多个FdEvent
    {
    private:
        /* data */
        int m_size {0};

        std::vector<FdEvent*> m_fd_group;
        
        Mutex m_mutex;
    public:
        FdEventGroup(int size);
        ~FdEventGroup();

        FdEvent* getFdEvent(int fd);

    public:
        static FdEventGroup* GetFdEventGroup();
    };
    
}




#endif