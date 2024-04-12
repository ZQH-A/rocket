#include "rocket/net/io_thread.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include <assert.h>

namespace rocket{

    IOThread::IOThread()
    {
        int rt = sem_init(&m_init_semaphore,0,0);
        assert(rt ==0);

        rt = sem_init(&m_start_semaphore,0,0);
        assert(rt ==0);

        pthread_create(&m_thread,NULL,&IOThread::Main,this); //this指针是传进去的参数

        //wait 直到新县城执行完main函数的前置 （就是唤醒等待线程的前面）
        sem_wait(&m_init_semaphore);
        
        DEBUGLOG("IOThread [%d] create success",m_thread_id);
    }

    IOThread::~IOThread()
    {
        m_event_loop->stop(); //停止loop循环
        sem_destroy(&m_init_semaphore);  //摧毁信号量
        sem_destroy(&m_start_semaphore);

        //pthread_join(m_thread,NULL);  //等待新创建的线程结束

        if(m_event_loop)  //删除指针并赋值为NULL
        {
            delete m_event_loop;
            m_event_loop = NULL;
        }
    }

    void* IOThread::Main(void *arg) //线程执行的函数
    {
        IOThread* thread = static_cast<IOThread*> (arg);

        thread->m_event_loop = new EventLoop();
        thread->m_thread_id = getThreadId();

        //唤醒等待的线程 
        sem_post(&thread->m_init_semaphore);

        //让IO线程等待，直到我们主动启动
        DEBUGLOG("IOThread %d created, wait start semphore",thread->m_thread_id);

        sem_wait(&thread->m_start_semaphore);
        DEBUGLOG("IOThread %d start loop",thread->m_thread_id);


        thread->m_event_loop->loop();

        DEBUGLOG("IOThread %d end loop",thread->m_thread_id);

        return NULL;
    }


    EventLoop* IOThread::getEventLoop()
    {
        return m_event_loop;
    }

    void IOThread::join()
    {
        pthread_join(m_thread,NULL);
    }

    void  IOThread::start() //主动唤醒新线程并执行循环
    {   
        DEBUGLOG("Now invoke IOThread %d",m_thread_id);
        sem_post(&m_start_semaphore);
    }
}