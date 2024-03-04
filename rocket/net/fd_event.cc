#include "rocket/net/fd_event.h"
#include "rocket/common/log.h"
#include <string.h>


namespace rocket{

    FdEvent::FdEvent(int fd):m_fd(fd)
    {
        memset(&m_listen_events,0,sizeof(m_listen_events));
    }


    FdEvent::~FdEvent()
    {

    }

    std::function<void()> FdEvent::hander(TriggerEvent event_type) 
    { //根据触发的读事件或写事件 通过Fd_event将 读回调函数或写回调函数加入到任务队列中
        if(event_type == TriggerEvent::IN_EVENT) //读事件执行读回调函数
        {
            return m_read_callback;
        }else{ //否则执行写回调函数
            return m_write_callback;
        }
    }


    void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback)
    {
        if(event_type == TriggerEvent::IN_EVENT)
        {
            m_listen_events.events |= EPOLLIN;
            m_read_callback = callback;
            m_listen_events.data.ptr = this; //有点印象 
            //将epoll_event中的指针指向整个Fd_event类 里面有读写回调函数
            //当epoll_wait中返回的有当前这个epoll_event时，
            //就可以直接用一个线程调用根据触发的读写事件调用对应的读写函数
        }else{
            m_listen_events.events |= EPOLLOUT;
            m_write_callback = callback;
            m_listen_events.data.ptr = this;
        }
    }
}