#ifndef ROCKET_NET_FDEVENT_H
#define ROCKET_NET_FDEVENT_H

#include <functional>
#include <sys/epoll.h>

namespace rocket{
    class FdEvent{

        public:

        enum TriggerEvent {
            IN_EVENT = EPOLLIN,
            OUT_EVENT = EPOLLOUT
        };

        FdEvent(int fd);

        FdEvent();

        ~FdEvent();

        std::function<void()> hander(TriggerEvent event_type);

        void listen(TriggerEvent event_type, std::function<void()> callback);

        int getFd() const {
            return m_fd;
        }

        epoll_event getEpollEvent() 
        {
            return m_listen_events;
        }

        

        protected:
        int m_fd {-1};  //监听的文件描述符

        epoll_event m_listen_events; //监听的事件

        std::function<void()> m_read_callback;  //读回调函数
        std::function<void()> m_write_callback; //写回调函数
    };
}


#endif