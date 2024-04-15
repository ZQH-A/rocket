#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

//reator模型：将套接字放在树上监听，当有套接字发生了变化，
//就用线程处理套接字到来的任务

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "rocket/common/mutex.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/wakeup_fd_event.h"
#include "rocket/net/timer.h"
#include "rocket/net/timer_event.h"

namespace rocket{

    class EventLoop{
        public:

            EventLoop();
            ~EventLoop();

            void loop();  //核心函数

            void wakeup(); 

            void stop(); //终止函数

            void addEpollEvent(FdEvent * event); //文件描述符上树
            void deleteEpollEvent(FdEvent * event); //文件描述符下树
            bool isInLoopThread(); //判断是否是当前线程添加和删除，如果是其他线程的话要加锁

            void addTask(std::function<void()> cb, bool is_wake_up = false); //把任务添加到 类里面
            void addTimerEvent(TimerEvent::s_ptr event);

        public: 
            static EventLoop* GetGurrentEventLoop(); //获取当前线程的eventloop对象
        private:

            void dealWakeup();
            void initWakeUpFdEvent();
            void initTimer();
        private:

            pid_t m_thread_id; //当前线程号
            std::set<int> m_listen_fds;//当前监听的所有套接字
            int m_epoll_fd {0};  // epoll 句柄
            int m_wakeup_fd {0}; //唤醒epoll_wait 
            WakeUpFdEvent* m_wakeup_fd_event {nullptr}; //唤醒epoll_wait
            bool m_stop_flag {false}; //stop标志

            std::queue<std::function<void()>> m_pending_tasks; //所有待执行的任务队列

            Mutex m_mutex; //封装的一个互斥锁

            Timer* m_timer {NULL};
    };

}



#endif