#include "rocket/net/eventloop.h"
#include <sys/socket.h>
#include <sys/eventfd.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include <sys/epoll.h>
#include <string.h>


//判断当前的fd是否已经上树  //如果已经在树上，  //添加失败
#define ADD_TO_EPOLL() \
        auto it = m_listen_fds.find(event->getFd()); \
        int op = EPOLL_CTL_ADD; \
        if(it != m_listen_fds.end()) \
        { \
            op = EPOLL_CTL_MOD; \
        } \
        epoll_event tmp = event->getEpollEvent(); \
        int rt = epoll_ctl(m_epoll_fd,op,event->getFd(),&tmp); \
        if(rt == -1)  \
        { \
            ERRORLOG("failed epoll_ctl when add fd, errno=%d, error info [%s]",errno,strerror(errno)); \
        } \
        DEBUGLOG("add event sucess, fd [%d]",event->getFd()); \

//下树 //没找到 直接返回 //找到了删除
#define DELETE_TO_EPOLL() \
        auto it = m_listen_fds.find(event->getFd()); \
        if(it == m_listen_fds.end()) \
        { \
            return; \
        } \
        int op = EPOLL_CTL_DEL; \
        epoll_event tmp = event->getEpollEvent(); \
        int rt = epoll_ctl(m_epoll_fd,op,event->getFd(),&tmp); \
        if(rt == -1) \
        { \
            ERRORLOG("failed epoll_ctl when delete fd, errno=%d, error info [%s]",errno,strerror(errno)); \
        } \
        DEBUGLOG("delete event sucess,fd[%d]",event->getFd()); \


namespace rocket{

    //用于判断当前线程是否创建过 Eventloop类
    static thread_local EventLoop* t_current_eventloop = NULL;  //线程局部变量
    static int g_epoll_max_timeout = 10000;  //最大退出时间
    static int g_epoll_max_events = 10;  //最大监听时间


    EventLoop::EventLoop()
    {
        //如果当前线程已经创建过，则 打印错误日志 并退出
        if(t_current_eventloop !=NULL)
        {
            ERRORLOG("failed to create event loop, this thread has created event loop");
            exit(0);
        }
        m_thread_id = getThreadId();


        m_epoll_fd = epoll_create(10); //创建树
        if(m_epoll_fd == -1)
        {
            ERRORLOG("failed to create event loop, epool_create error, error info [%d]",errno);
            exit(0);
        }
        //初始化Wakeupfd
        initWakeUpFdEvent();
        //初始化定时器fd
        initTimer();

        INFOLOG("success create eventloop in thread %d",m_thread_id);
        t_current_eventloop = this;
    }
    EventLoop::~EventLoop()
    {
        close(m_epoll_fd);
        if(m_wakeup_fd_event)
        {
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = nullptr;
        }

        if(m_timer)
        {
            delete m_timer;
            m_timer = NULL;
        }
    }

    void EventLoop::initWakeUpFdEvent()
    {
        //当有事件来临时，就会马上唤醒处理epoll_wait
        m_wakeup_fd = eventfd(0, EFD_NONBLOCK);   //专门用于事件通知的文件描述符
        if(m_wakeup_fd < 0)
        {
            ERRORLOG("failed to create wakeupfd, eventfd create error, error info [%d]",errno);
            exit(0);
        }
        m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
        m_wakeup_fd_event->listen(FdEvent::IN_EVENT,[this](){  //设置wakeupfd的读回调函数
            char buf[8];
            while(read(m_wakeup_fd_event->getFd(), buf, 8) != -1 && errno != EAGAIN) 
            { //当错误是EAGAIN时 或者read函数返回的是 -1，说明已经读完了
            }
            DEBUGLOG("read full bytes from wakeup fd[%d]",m_wakeup_fd_event);}
        );
        addEpollEvent(m_wakeup_fd_event);  //将wakeupfd上树
    } 

    void EventLoop::initTimer()  //初始化Timer
    {
        m_timer = new Timer();
        addEpollEvent(m_timer);
    }

    void EventLoop::addTimerEvent(TimerEvent::s_ptr event)
    {
        m_timer->addTimerEvent(event);
    }

    void EventLoop::loop()  //核心函数
    {
        while(!m_stop_flag)
        {   //当stop标志是false时，说明loop函数一直在循环，然后需要从任务队列中取任务
            //当有多个线程同时取时，需要考虑互斥
            ScopeMutext<Mutex> lock(m_mutex);
            std::queue<std::function<void()>> tmp_tasks;
            m_pending_tasks.swap(tmp_tasks); //交换两个队列
            lock.unlock();

             while(!tmp_tasks.empty())
             {
                std::function<void()> cb = tmp_tasks.front(); //调用任务队列中的函数
                tmp_tasks.pop(); 
                if(cb)  //判断 回调函数是否是空
                {
                    cb();
                }
             }

             //如果由定时任务需要执行，那么执行
             //1.怎么判断是否需要执行
             //2.怎么去监听
            

             int timeout = g_epoll_max_timeout;
             epoll_event result_events[g_epoll_max_events]; //套接字数组

             

             //返回的是变换的文件描述符个数
             int rt = epoll_wait(m_epoll_fd,result_events,g_epoll_max_events,timeout);
             DEBUGLOG("now begin to epoll_wait,rt = %d",rt);

            if(rt < 0 ) //说明epoll出问题
            {
                ERRORLOG("eploo_wait error , error=",errno);
            }else{ // 否则返回的是变换套接字个数，遍历所有套接字
                for(int i = 0; i < rt; ++i)
                {
                    epoll_event trigger_event = result_events[i]; //遍历返回变化的epoll_event
                    FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);
                    if(fd_event == nullptr)  //返回的是空直接跳过
                    {
                        continue;
                    }
                    if(trigger_event.events & EPOLLIN) //触发可读事件
                    {  //将读回调函数加入到任务队列中
                        DEBUGLOG("fd %d trigger EPOLLIN event",fd_event->getFd());
                        addTask(fd_event->hander(FdEvent::IN_EVENT));
                    }

                    if(trigger_event.events & EPOLLOUT) //触发写事件
                    { //将写回调函数加入到任务队列中
                        DEBUGLOG("fd %d trigger EPOLLOUT event",fd_event->getFd());
                        addTask(fd_event->hander(FdEvent::OUT_EVENT));
                    }

                }
            }
        }
    }

    void EventLoop::wakeup()
    {
        m_wakeup_fd_event->wakeup();
    }

    void EventLoop::stop() //终止函数
    {
        m_stop_flag = true;
    }

    void EventLoop::dealWakeup()
    {

    }


    void EventLoop::addEpollEvent(FdEvent * event) //文件描述符上树
    {
        DEBUGLOG("addEpollEvent.........");
        if(isInLoopThread())  //是当前线程执行 直接添加fd 
        {
            ADD_TO_EPOLL();
        } else { //不是同一个线程，为什么要加入到 任务队列中，不是很懂？
        //作者的解释 是说 当主线程调用其他的线程 执行这个函数，就是不是主线程执行时
        //会跳转到这里来，然后将添加fd的任务 添加到任务队列中去，等到主线程被唤醒时
        //然后取任务队列中的任务时 执行 添加fd的任务 （删除fd的任务同理）
        //我的理解： 只能这个线程或者主线程执行 添加fd或者删除fd的任务，其他线程执行时
        //就加到任务队列中，还是让主线程执行
            auto cb = [this, event](){
                ADD_TO_EPOLL();
            };
            addTask(cb,true);
        }
    }
     
    void EventLoop::deleteEpollEvent(FdEvent * event) //文件描述符下树
    {
        if(isInLoopThread()) //是同一个线程
        {
            DELETE_TO_EPOLL();
        }else{   //不是同一个线程 为什么要加入到任务队列中
            auto cb = [this,event](){
                DELETE_TO_EPOLL();
            };
            addTask(cb,true);
        }
    }

    bool EventLoop::isInLoopThread() //判断是否是EventLoop当前线程
    {
        return getThreadId() == m_thread_id;
    }


    void EventLoop::addTask(std::function<void()> cb,bool is_wake_up /*false*/) //把任务添加到 任务队列里面
    {  //is_wake_up 默认是false
        ScopeMutext<Mutex> lock(m_mutex);
        m_pending_tasks.push(cb);
        lock.unlock();
        if(is_wake_up)
        {
            wakeup(); //唤醒 什么 暂时不知道
        }
    }

}